#include "tlb.h"
#include "tlb/tlb_0327.h"

extern Obj *gBattleTask; /* the battle's task (beffect.c) */
extern CamView *gBattleCamView; /* the battle's camera (beffect.c) */

#define cosf (*cosf)
#define sinf (*sinf)

/* A print the original left in: keeps its string, emits no code. */
#define DPRINTF

/* acollAtkAddYZ and acollDmgAddYZ return their box (main declares them void). */
typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

/* The starters below each put a position and a type in their task's argument; each argument
 * holds only those (the rest of it is the task's own). */

typedef struct OnibiArg {
    /* 0x00 */ u8 pad00[0x28];
    /* 0x28 */ f32 pos[3];
    /* 0x34 */ s32 type;
    /* 0x38 */ u8 pad38[0xC];
} OnibiArg;

/* "Onibi"'s work. */
typedef struct OnibiW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ ACollFrame *atk;
    /* 0x14 */ ACollFrame *dmg;
    /* 0x18 */ Anim anim; /* the flame's texture animation */
    /* 0x28 */ u8 pad28[0xC];
    /* 0x34 */ s32 type; /* 0: still; 1-3: fly to the boss's side, then at the Impact; 4: the demo's */
    /* 0x38 */ s32 timer;
    /* 0x3C */ s32 alpha;
    /* 0x40 */ s32 state; /* 4: put out */
} OnibiW;

extern u8 bdevOnibiDl[]; /* the flame's display list, in file 0x125 */
extern u8 onibiFlameTex0[];
extern u8 onibiFlameTex2[];
extern u8 onibiFlameTex5[];
extern u8 pentaniRingTex1[];
extern u8 bisuGTex0[];
extern u8 bisuGTex1[];
AnimFrame onibiFlameAnim[] = {
    { (u32) onibiFlameTex0, 2 },
    { 0x03000D80, 2 },
    { (u32) onibiFlameTex2, 2 },
    { 0x03001D80, 2 },
    { 0x03002580, 2 },
    { (u32) onibiFlameTex5, 2 },
    { 0x03003580, 2 },
    { 0x03003D80, 2 },
    { 0x03004580, 2 },
    { 0x03004D80, 2 },
    { 0x03005580, 2 },
    { 0x03005D80, 2 },
    { 0x03006580, 2 },
    { 0x03006D80, 2 },
    { 0x03007580, 2 },
    { 0x03007D80, 2 },
    { 0, 0 },
}; /* the flame's texture animation */

/* "Onibi" (a will-o'-the-wisp): file 0x125's flame at the position given, ten times life size
 * (types 1-3: seven), with an attack box (radius 30, kind 0x6E) and a damage box (30 by 60, 30
 * down, kind 0x6F). Type 0 fades in by 20 and out by 0.5; type 4 fades in by 5, burns 600
 * frames and fades out by 1; types 1-3 fade in, fly at speed 10 to 500 below, right or left of
 * the boss, wait 420 frames and fly at speed 30 at the Impact, ending there. Landing its attack
 * or hit by kinds 1, 3, 5, 8, 11 or 15 (coins for 1-8, sound 0x2A3) it goes out: both boxes
 * off, it stops and fades by 5. Types 0 and 4 end on gBattleFlags2 8, types 1-3 on 0x400. */
s32 bdevOnibiProc(Obj *obj, s16 msg, void *arg) {
    OnibiW *w = (OnibiW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Onibi";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bdevOnibiDl, 0x125);
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x27);
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 30, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x6E;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 30, 60, -30, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0x6F;
            w->type = ((OnibiArg *) arg)->type;
            fadeAnimStart(&w->anim, onibiFlameAnim, 0, &w->frame->segs[3], floadGetSetFile(0x125, 0));
            if (w->type == 1 || w->type == 2 || w->type == 3) {
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 7.0f;
            } else {
                w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 10.0f;
            }
            w->frame->pos[0] = ((OnibiArg *) arg)->pos[0];
            w->frame->pos[1] = ((OnibiArg *) arg)->pos[1];
            w->frame->pos[2] = ((OnibiArg *) arg)->pos[2];
            w->alpha = 0;
            w->timer = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
            break;
        case 0:
            if ((w->type == 0 || w->type == 4) && (gBattleFlags2 & 8)) {
                return -1;
            }
            if ((w->type == 1 || w->type == 2 || w->type == 3) && (gBattleFlags2 & 0x400)) {
                return -1;
            }
            fadeAnimStep(&w->anim);
            if (acollAtkWasHit(w->atk) == 1) {
                w->atk->flags &= ~2;
                w->dmg->flags &= ~2;
                w->state = 4;
            }
            if (acollDmgWasHit(w->dmg)
                && (w->dmg->hitKind == 15 || w->dmg->hitKind == 1 || w->dmg->hitKind == 3 || w->dmg->hitKind == 5
                    || w->dmg->hitKind == 8 || w->dmg->hitKind == 11)) {
                if (w->dmg->hitKind == 1 || w->dmg->hitKind == 3 || w->dmg->hitKind == 5 || w->dmg->hitKind == 8) {
                    ryouCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                }
                w->atk->flags &= ~2;
                w->dmg->flags &= ~2;
                SoundLocation2(0x2A3, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 300.0f,
                               15000.0f, gBattleCamView->fovy);
                w->state = 4;
            }
            if (w->type == 0) {
                switch (w->state) {
                    case 0:
                        w->alpha += 20;
                        if (w->alpha > 255) {
                            w->alpha = 255;
                            w->state++;
                        }
                        break;
                    case 1:
                        w->alpha -= 0.5;
                        if (w->alpha <= 0) {
                            w->alpha = 0;
                            return -1;
                        }
                        break;
                    case 4:
                        w->atk->flags &= ~2;
                        w->dmg->flags &= ~2;
                        w->alpha -= 5;
                        if (w->alpha < 0) {
                            w->alpha = 0;
                            return -1;
                        }
                        break;
                }
            }
            if (w->type == 4) {
                switch (w->state) {
                    case 0:
                        w->alpha += 5;
                        if (w->alpha > 255) {
                            w->alpha = 255;
                            w->state++;
                        }
                        break;
                    case 1:
                        if (w->timer++ > 600) {
                            w->state++;
                        }
                        break;
                    case 2:
                        w->alpha--;
                        if (w->alpha <= 0) {
                            w->alpha = 0;
                            return -1;
                        }
                        break;
                    case 4:
                        w->atk->flags &= ~2;
                        w->dmg->flags &= ~2;
                        w->alpha -= 5;
                        if (w->alpha < 0) {
                            w->alpha = 0;
                            return -1;
                        }
                        break;
                }
            } else if (w->type == 1 || w->type == 2 || w->type == 3) {
                bossApplyVel(obj);
                switch (w->state) {
                    case 0:
                        w->alpha += 20;
                        if (w->alpha > 255) {
                            w->alpha = 255;
                            w->state++;
                        }
                        break;
                    case 1:
                        if (w->type == 1 && !BmoveX(obj, gBossPosX, gBossPosY - 500.0f, gBossPosZ + 500.0f, 10.0f)) {
                            w->state++;
                        }
                        if (w->type == 2
                            && !BmoveX(obj, gBossPosX + 500.0f, gBossPosY + 500.0f, gBossPosZ + 500.0f, 10.0f)) {
                            w->state++;
                        }
                        if (w->type == 3
                            && !BmoveX(obj, gBossPosX - 500.0f, gBossPosY + 500.0f, gBossPosZ + 500.0f, 10.0f)) {
                            w->state++;
                        }
                        break;
                    case 2:
                        if (w->timer++ > 420) {
                            BmoveX(obj, gImpactPosX, gImpactPosY, gImpactPosZ, 30.0f);
                            w->state++;
                        }
                        break;
                    case 3:
                        if (!BmoveX(obj, gImpactPosX, gImpactPosY, gImpactPosZ, 30.0f)) {
                            return -1;
                        }
                        break;
                    case 4:
                        w->atk->flags &= ~2;
                        w->dmg->flags &= ~2;
                        w->vel[0] = 0.0f;
                        w->vel[1] = 0.0f;
                        w->alpha -= 5;
                        w->vel[2] = 0.0f;
                        if (w->alpha < 0) {
                            w->alpha = 0;
                            return -1;
                        }
                        break;
                }
            }
            break;
        case 3:
            if (w->type == 0 || w->type == 4) {
                w->frame->dl = bossPrimEnvColorDL(100, 0, 255, 255, 0, 100, w->alpha);
            } else {
                w->frame->dl = bossPrimEnvColorDL(0, 255, 100, 100, 0, 255, w->alpha);
            }
            DPRINTF("[onibiCreate]タスクの作成に失敗しました。\n");
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An "Onibi" (will-o'-the-wisp) of a type at x, y, z under the effect task. */
void onibiCreate(f32 x, f32 y, f32 z, s32 type) {
    OnibiArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bdevOnibiProc, gBattleTask->file, &arg);
}

/* "Penta", "Pentain" and "CurseBall"'s argument. */
typedef struct PentaArg {
    /* 0x00 */ u8 pad00[0x14];
    /* 0x14 */ f32 pos[3];
    /* 0x20 */ s32 type;
    /* 0x24 */ u8 pad24[0xC];
} PentaArg;

/* "Penta" and "Pentain"'s work. */
typedef struct PentaW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ u8 pad04[0x20];
    /* 0x24 */ s32 step; /* 0: fading in; 1: fading out */
    /* 0x28 */ u8 pad28[0x4];
    /* 0x2C */ s32 alpha;
} PentaW;

extern u8 bdevPentaDl[]; /* the pentagram's two display lists, in file 0xF0 */
extern u8 bdevPentainDl[];

/* "Penta": a pentagram from file 0xF0 lying flat at the position given, spreading by 5 a frame
 * while it fades in by 10 to 255 and out again, blue on white. */
s32 bdevPentaProc(Obj *obj, s16 msg, void *arg) {
    PentaW *w = (PentaW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Penta";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                PRINTF("[pentaCreate]タスクの作成に失敗しました。\n");
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bdevPentaDl, 0xF0);
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x20);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 1.0f;
            w->frame->pos[0] = ((PentaArg *) arg)->pos[0];
            w->frame->pos[1] = ((PentaArg *) arg)->pos[1];
            w->frame->pos[2] = ((PentaArg *) arg)->pos[2];
            w->frame->rot[0] = -1.5707964f;
            w->alpha = 0;
            w->step = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            w->frame->scale[2] += 5.0;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            switch (w->step) {
                case 0:
                    w->alpha += 10;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                        w->step++;
                    }
                    break;
                case 1:
                    w->alpha -= 10;
                    if (w->alpha < 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    break;
                case 2:
                case 3:
                    break;
            }
            break;
        case 3:
            w->frame->dl = bossPrimEnvColorDL(0, 0, 255, 255, 255, 255, w->alpha);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Penta" of a type at x, y, z under the effect task. */
void pentaCreate(f32 x, f32 y, f32 z, s32 type) {
    PentaArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bdevPentaProc, gBattleTask->file, &arg);
}

/* "Pentain": a pentagram from file 0xF0, 50 times life size, lying flat at the position given,
 * fading in by 5 to 255 and out by 1, blinking white every fourth frame. */
s32 bdevPentainProc(Obj *obj, s16 msg, void *arg) {
    PentaW *w = (PentaW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Pentain";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                PRINTF("[pentainCreate]タスクの作成に失敗しました。\n");
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bdevPentainDl, 0xF0);
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x20);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 50.0f;
            w->frame->pos[0] = ((PentaArg *) arg)->pos[0];
            w->frame->pos[1] = ((PentaArg *) arg)->pos[1];
            w->frame->pos[2] = ((PentaArg *) arg)->pos[2];
            w->frame->rot[0] = -1.5707964f;
            w->alpha = 0;
            w->step = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            switch (w->step) {
                case 0:
                    w->alpha += 5;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                        w->step++;
                    }
                    break;
                case 1:
                    w->alpha--;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    break;
                case 2:
                case 3:
                    break;
            }
            break;
        case 3:
            if ((gGameFrameCount & 3) == 2) {
                w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->alpha);
            } else {
                w->frame->dl = bossPrimEnvColorDL(255, 255, 100, 255, 0, 0, w->alpha);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Pentain" of a type at x, y, z under the effect task. */
void pentainCreate(f32 x, f32 y, f32 z, s32 type) {
    PentaArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bdevPentainProc, gBattleTask->file, &arg);
}

/* "CurseBall"'s work. */
typedef struct CurseBallW {
    /* 0x00 */ PolFrame *ball;
    /* 0x04 */ u8 pad04[0xC];
    /* 0x10 */ PolFrame *ring; /* its picture by the type */
    /* 0x14 */ u8 pad14[0xC];
    /* 0x20 */ s32 type;
    /* 0x24 */ s32 step; /* 0: fading in; 1: fading out */
    /* 0x28 */ s32 time;
    /* 0x2C */ s32 alpha;
} CurseBallW;

extern u8 bdevCurseBallDl[]; /* the curse ball and its four rings, in file 0xF0 */
extern u8 bdevCurseBallRing0Dl[];
extern u8 bdevCurseBallRing1Dl[];
extern u8 bdevCurseBallRing2Dl[];
extern u8 bdevCurseBallRing3Dl[];

/* "CurseBall": a ball from file 0xF0 ten times life size at the position given, with a ring
 * (one of four by the type) 7.5 times. Both fade in by 3 to 128 and out by 1, the ball blinking
 * white every ninth frame and the ring three frames later. */
s32 bdevCurseBallProc(Obj *obj, s16 msg, void *arg) {
    CurseBallW *w = (CurseBallW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CurseBall";
            w->ball = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ball == NULL) {
                PRINTF("[pentainCreate]タスクの作成に失敗しました。\n");
                return -1;
            }
            w->ring = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring == NULL) {
                return -1;
            }
            bossSetPolDL(w->ball, (Gfx *) bdevCurseBallDl, 0xF0);
            bossSetFrameFlags((AnimeFrame *) w->ball, 0x27);
            w->type = ((PentaArg *) arg)->type;
            if (w->type == 0) {
                bossSetPolDL(w->ring, (Gfx *) bdevCurseBallRing0Dl, 0xF0);
            }
            if (w->type == 1) {
                bossSetPolDL(w->ring, (Gfx *) bdevCurseBallRing1Dl, 0xF0);
            }
            if (w->type == 2) {
                bossSetPolDL(w->ring, (Gfx *) bdevCurseBallRing2Dl, 0xF0);
            }
            if (w->type == 3) {
                bossSetPolDL(w->ring, (Gfx *) bdevCurseBallRing3Dl, 0xF0);
            }
            bossSetFrameFlags((AnimeFrame *) w->ring, 0x27);
            w->ball->scale[0] = w->ball->scale[1] = w->ball->scale[2] = 10.0f;
            w->ball->pos[0] = ((PentaArg *) arg)->pos[0];
            w->ball->pos[1] = ((PentaArg *) arg)->pos[1];
            w->ball->pos[2] = ((PentaArg *) arg)->pos[2];
            w->ring->scale[0] = w->ring->scale[1] = w->ring->scale[2] = 7.5f;
            w->ring->pos[0] = w->ball->pos[0];
            w->ring->pos[1] = w->ball->pos[1];
            w->ring->pos[2] = w->ball->pos[2];
            w->time = 0;
            w->alpha = 0;
            w->step = 0;
            break;
        case 1:
            frmDestroy(&w->ball->f);
            frmDestroy(&w->ring->f);
            break;
        case 0:
            switch (w->step) {
                case 0:
                    w->alpha += 3;
                    if (w->alpha > 128) {
                        w->alpha = 128;
                        w->step++;
                    }
                    break;
                case 1:
                    w->alpha--;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    break;
                case 2:
                case 3:
                    break;
            }
            w->time++;
            break;
        case 3:
            if (w->time % 9 == 3) {
                w->ball->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->alpha);
            } else {
                w->ball->dl = bossPrimEnvColorDL(200, 200, 255, 255, 255, 255, w->alpha);
            }
            if (w->time % 9 == 6) {
                w->ring->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->alpha);
            } else {
                w->ring->dl = bossPrimEnvColorDL(255, 255, 100, 255, 0, 0, w->alpha);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "CurseBall" of a type at x, y, z under the effect task. */
void bdevCurseBallCreate(f32 x, f32 y, f32 z, s32 type) {
    PentaArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bdevCurseBallProc, gBattleTask->file, &arg);
}

typedef struct IronBallArg {
    /* 0x00 */ u8 pad00[0x18];
    /* 0x18 */ f32 pos[3];
    /* 0x24 */ s32 type;
    /* 0x28 */ u8 pad28[0xC];
} IronBallArg;

/* "IronBall"'s work. */
typedef struct IronBallW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ f32 vel[3]; /* BmoveX's and bossApplyVel's (bmove.c) */
    /* 0x10 */ ACollFrame *atk;
    /* 0x14 */ ACollFrame *dmg;
    /* 0x18 */ u8 pad18[0xC];
    /* 0x24 */ s32 type;
    /* 0x28 */ s32 state; /* 0: flying at the Impact; 1: back to the boss; 2, 3: waiting */
    /* 0x2C */ s32 time;
} IronBallW;

extern u8 bdevIronBallDl[]; /* the iron ball's display list, in file 0xF0 */

/* "IronBall": file 0xF0's iron ball at the position given, three times the size, spinning, with
 * an attack box (radius 60, kind 0x6C) and a damage box (70 by 140, 70 down, kind 0x6D). It
 * flies at speed 80 at the Impact. Landing its attack, or hit by kinds 1, 3, 5, 8 or 11 (a
 * puff, coins for all but 11, sound 0x14F unless the battle is ending), it flies back to the
 * boss at speed 90 and ends there. It ends at once on gBattleFlags2 0x400. */
s32 bdevIronBallProc(Obj *obj, s16 msg, void *arg) {
    IronBallW *w = (IronBallW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "IronBall";
            w->frame = (PolFrame *) frmCreate(gFrmModelLayer, gFrmKindPol);
            if (w->frame == NULL) {
                return -1;
            }
            bossSetPolDL(w->frame, (Gfx *) bdevIronBallDl, 0xF0);
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x23);
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 60, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x6C;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 70, 140, -70, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0x6D;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 3.0f;
            w->frame->pos[0] = ((IronBallArg *) arg)->pos[0];
            w->frame->pos[1] = ((IronBallArg *) arg)->pos[1];
            w->frame->pos[2] = ((IronBallArg *) arg)->pos[2];
            w->type = ((IronBallArg *) arg)->type;
            w->time = 0;
            BmoveX(obj, gImpactPosX, gImpactPosY, gImpactPosZ, 80.0f);
            w->state = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
            break;
        case 0:
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            w->frame->rot[2] += 0.209439516046560942;
            bossApplyVel(obj);
            if (acollAtkWasHit(w->atk) == 1) {
                itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                w->state = 1;
            }
            switch (w->state) {
                case 0:
                    if (acollDmgWasHit(w->dmg)) {
                        if (w->dmg->hitKind == 1 || w->dmg->hitKind == 3 || w->dmg->hitKind == 5
                            || w->dmg->hitKind == 8 || w->dmg->hitKind == 11) {
                            itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                            if (w->dmg->hitKind == 1 || w->dmg->hitKind == 3 || w->dmg->hitKind == 5
                                || w->dmg->hitKind == 8) {
                                ryouCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                            }
                            if (!(gBattleFlags & 0x10)) {
                                SoundLocation2(0x14F, gBattleCamView, w->dmg->hitPos[0], w->dmg->hitPos[1],
                                               w->dmg->hitPos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            }
                            w->state = 1;
                        }
                    }
                    break;
                case 1:
                    if (BmoveX(obj, gBossPosX, gBossPosY + 450.0f, gBossPosZ, 90.0f) == 0) {
                        return -1;
                    }
                    w->atk->flags &= ~2;
                    break;
                case 2:
                    if (acollDmgWasHit(w->dmg)) {
                        if (w->dmg->hitKind == 15 || w->dmg->hitKind == 1 || w->dmg->hitKind == 3
                            || w->dmg->hitKind == 5 || w->dmg->hitKind == 8 || w->dmg->hitKind == 11) {
                            itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                            if (w->dmg->hitKind == 1 || w->dmg->hitKind == 3 || w->dmg->hitKind == 5
                                || w->dmg->hitKind == 8) {
                                ryouCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                            }
                            w->state = 1;
                        }
                    }
                    if (w->time > 20) {
                        BmoveX(obj, gImpactPosX, gImpactPosY, gImpactPosZ, 80.0f);
                        w->state = 0;
                    }
                    break;
                case 3:
                    if (acollDmgWasHit(w->dmg)) {
                        if (w->dmg->hitKind == 15 || w->dmg->hitKind == 1 || w->dmg->hitKind == 3
                            || w->dmg->hitKind == 5 || w->dmg->hitKind == 8 || w->dmg->hitKind == 11) {
                            itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                            if (w->dmg->hitKind == 1 || w->dmg->hitKind == 3 || w->dmg->hitKind == 5
                                || w->dmg->hitKind == 8) {
                                ryouCreate(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 2.0f);
                            }
                            w->state = 1;
                        }
                    }
                    if (w->time > 20) {
                        BmoveX(obj, gImpactPosX, gImpactPosY, gImpactPosZ, 80.0f);
                        w->state = 0;
                    }
                    break;
            }
            w->time++;
            DPRINTF("バルカンに当たったけど無視。\n");
            DPRINTF("[ironballCreate]タスクの作成に失敗しました。\n");
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* An "IronBall" of a type at x, y, z under the effect task. */
void ironballCreate(f32 x, f32 y, f32 z, s32 type) {
    IronBallArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bdevIronBallProc, gBattleTask->file, &arg);
}

/* The two "Pentani" tasks' argument. */
typedef struct PentaniArg {
    /* 0x00 */ u8 pad00[0x20];
    /* 0x20 */ f32 pos[3];
    /* 0x2C */ s32 type;
    /* 0x30 */ u8 pad30[0xC];
} PentaniArg;

/* "Pentani"'s work. */
typedef struct PentaniW {
    /* 0x00 */ AnimeFrame *body;
    /* 0x04 */ PolFrame *ring[2]; /* turning opposite ways, their texture animated */
    /* 0x0C */ PolFrame *shadow; /* the pentagram on the floor, drawn depth only */
    /* 0x10 */ Anim anim; /* the rings' texture animation */
    /* 0x20 */ u8 pad20[0x10];
    /* 0x30 */ s32 state;
    /* 0x34 */ s32 time;
    /* 0x38 */ s32 alpha;
} PentaniW;

extern u8 bdevPentaniBodyAnim[]; /* the body's animation, in file 0xF0 */
extern u8 bdevPentaniRingDl[]; /* the ring's display list, in file 0xF0 */
AnimFrame pentaniRingAnim[] = {
    { 0x03016DF0, 2 },
    { (u32) pentaniRingTex1, 2 },
    { 0x030175F0, 2 },
    { 0x030179F0, 2 },
    { 0x03017DF0, 2 },
    { 0x030181F0, 2 },
    { 0x030185F0, 2 },
    { 0x030181F0, 2 },
    { 0x03017DF0, 2 },
    { 0x030179F0, 2 },
    { 0x030175F0, 2 },
    { (u32) pentaniRingTex1, 2 },
    { 0, 0 },
}; /* the rings' texture animation */

/* "Pentani": an animated body from file 0xF0, 60 times life size, at the position given, turning
 * by a degree a frame, with two rings turning opposite ways and a pentagram's shadow 50 below.
 * It fades in by 5 while its animation plays; at frame 181 the rings grow until 10 high; at
 * frame 661 it fades out by 5 and ends. */
s32 bdevPentaniProc(Obj *obj, s16 msg, void *arg) {
    PentaniW *w = (PentaniW *) FX(obj);
    s32 pad[4];

    switch (msg) {
        case 2:
            obj->name = "Pentani";
            w->shadow = (PolFrame *) frmCreatePrev(gFrmLayer2, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            w->body = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
            if (w->body == NULL) {
                return -1;
            }
            w->ring[0] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring[0] == NULL) {
                return -1;
            }
            w->ring[1] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring[1] == NULL) {
                return -1;
            }
            bossSetAnimAt(w->body, bdevPentaniBodyAnim, 0xF0, 0.0f);
            bossSetFrameFlags2(w->body, 0x20);
            bossSetPolDL(w->ring[0], (Gfx *) bdevPentaniRingDl, 0xF0);
            fadeAnimStart(&w->anim, pentaniRingAnim, 0, &w->ring[0]->segs[3], floadGetSetFile(0xF0, 0));
            bossSetFrameFlags((AnimeFrame *) w->ring[0], 0x20);
            bossSetPolDL(w->ring[1], (Gfx *) bdevPentaniRingDl, 0xF0);
            fadeAnimStart(&w->anim, pentaniRingAnim, 0, &w->ring[1]->segs[3], floadGetSetFile(0xF0, 0));
            bossSetFrameFlags((AnimeFrame *) w->ring[1], 0x20);
            bossSetPolDL(w->shadow, (Gfx *) bdevPentaDl, 0xF0);
            bossSetFrameFlags((AnimeFrame *) w->shadow, 0x20);
            w->body->scale[0] = w->body->scale[1] = w->body->scale[2] = 60.0f;
            w->body->pos[0] = ((PentaniArg *) arg)->pos[0];
            w->body->pos[1] = ((PentaniArg *) arg)->pos[1];
            w->body->pos[2] = ((PentaniArg *) arg)->pos[2];
            w->ring[0]->scale[0] = w->ring[0]->scale[1] = w->ring[0]->scale[2] = 0.0f;
            w->ring[0]->pos[0] = ((PentaniArg *) arg)->pos[0];
            w->ring[0]->pos[1] = ((PentaniArg *) arg)->pos[1];
            w->ring[0]->pos[2] = ((PentaniArg *) arg)->pos[2];
            w->ring[1]->scale[0] = w->ring[1]->scale[1] = w->ring[1]->scale[2] = 0.0f;
            w->ring[1]->pos[0] = ((PentaniArg *) arg)->pos[0];
            w->ring[1]->pos[1] = ((PentaniArg *) arg)->pos[1];
            w->ring[1]->pos[2] = ((PentaniArg *) arg)->pos[2];
            w->shadow->scale[0] = 50.0f;
            w->shadow->scale[1] = 50.0f;
            w->shadow->scale[2] = 70.0f;
            w->shadow->pos[0] = ((PentaniArg *) arg)->pos[0];
            w->shadow->pos[1] = ((PentaniArg *) arg)->pos[1] - 50.0f;
            w->shadow->pos[2] = ((PentaniArg *) arg)->pos[2];
            w->shadow->rot[0] = -1.5707964f;
            w->alpha = 0;
            w->state = 0;
            break;
        case 1:
            frmDestroy(&w->body->f);
            frmDestroy(&w->ring[0]->f);
            frmDestroy(&w->ring[1]->f);
            frmDestroy(&w->shadow->f);
            break;
        case 0:
            switch (w->state) {
                case 0:
                    w->body->rot[1] += 0.0174532930038800785;
                    bossStepAnim(w->body, 0.3f, 2);
                    w->alpha += 5;
                    if (w->alpha > 255) {
                        w->state++;
                    }
                    break;
                case 1:
                    w->body->rot[1] += 0.0174532930038800785;
                    w->alpha += 5;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                    }
                    if (bossStepAnim(w->body, 0.3f, 2)) {
                        w->state++;
                    }
                    break;
                case 2:
                    w->body->rot[1] += 0.0174532930038800785;
                    if (w->time > 180) {
                        w->state++;
                    }
                    break;
                case 3:
                    w->body->rot[1] += 0.0174532930038800785;
                    fadeAnimStep(&w->anim);
                    w->ring[0]->scale[2] = 6.8f;
                    w->ring[0]->scale[0] = w->ring[0]->scale[2];
                    w->ring[0]->scale[1] += 0.1;
                    w->ring[1]->scale[2] = 6.8f;
                    w->ring[1]->scale[0] = w->ring[1]->scale[2];
                    w->ring[1]->scale[1] += 0.1;
                    w->ring[0]->rot[1] += 0.00872664650194003924;
                    w->ring[1]->rot[1] -= 0.00872664650194003924;
                    if (w->ring[0]->scale[1] > 10.0) {
                        w->state++;
                    }
                    break;
                case 4:
                    fadeAnimStep(&w->anim);
                    w->body->rot[1] += 0.0174532930038800785;
                    w->ring[0]->rot[1] += 0.00872664650194003924;
                    w->ring[1]->rot[1] -= 0.00872664650194003924;
                    if (w->time > 660) {
                        w->state++;
                    }
                    break;
                case 5:
                    fadeAnimStep(&w->anim);
                    w->body->rot[1] += 0.0174532930038800785;
                    w->ring[0]->rot[1] += 0.00872664650194003924;
                    w->ring[1]->rot[1] -= 0.00872664650194003924;
                    w->alpha -= 5;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    break;
            }
            DPRINTF("pentimer[%d]\n");
            w->time++;
            break;
        case 3:
            if (!(gGameFrameCount & 3)) {
                w->body->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->alpha);
            } else {
                w->body->dl = bossPrimEnvColorDL(255, 255, 100, 255, 0, 0, w->alpha);
            }
            w->ring[0]->dl = bossPrimEnvColorDL(255, 255, 100, 255, 0, 0, w->alpha / 3);
            w->ring[1]->dl = bossPrimEnvColorDL(100, 255, 255, 255, 0, 0, w->alpha / 3);
            w->shadow->dl = gBossDepthOnlyDl;
            DPRINTF("[pentaniCreate]タスクの作成に失敗しました。\n");
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "Pentani" (bdevPentaniProc) of a type at x, y, z under the effect task. */
void pentaniCreate(f32 x, f32 y, f32 z, s32 type) {
    PentaniArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bdevPentaniProc, gBattleTask->file, &arg);
}

/* The create argument of the "Kesaeffect" task (bdevKesaeffectProc). */
typedef struct KesaEffectArg {
    /* 0x00 */ u8 pad00[8];
    /* 0x08 */ s32 type;
    /* 0x0C */ u8 pad0C[4];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ u8 pad1C[4];
} KesaEffectArg;

/* "Kesaeffect"'s work. */
typedef struct KesaEffectW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ ACollFrame *atk;
    /* 0x08 */ u8 pad08[0x4];
    /* 0x0C */ s32 time;
    /* 0x10 */ u8 pad10[0xC];
    /* 0x1C */ f32 alpha;
} KesaEffectW;

extern u8 bdevKesaeffectDl[]; /* the slash's display list, in file 0xF0 */

/* "Kesaeffect" (the source calls it kesaEffect, a diagonal slash): file 0xF0's slash at the
 * position given, with an attack box (radius 600, kind 0x67) that is never live: each frame
 * clears its live bit. It fades from 255 by 10, yellow, white below 50. It ends at once when
 * the battle ends. */
s32 bdevKesaeffectProc(Obj *obj, s16 msg, void *arg) {
    KesaEffectW *w = (KesaEffectW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "Kesaeffect";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                PRINTF("[kesaEffectCreate]隊長,タスクの作成に失敗しました。\n");
                return -1;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 600, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x67;
            bossSetPolDL(w->frame, (Gfx *) bdevKesaeffectDl, 0xF0);
            w->frame->pos[0] = ((KesaEffectArg *) arg)->pos[0];
            w->frame->pos[1] = ((KesaEffectArg *) arg)->pos[1];
            w->frame->pos[2] = ((KesaEffectArg *) arg)->pos[2];
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x20);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 1.0f;
            w->time = 0;
            w->alpha = 255.0f;
            break;
        case 0:
            if (gBattleFlags & 0x10) {
                return -1;
            }
            if (gBattleFlags2 & 0x4000) {
                return -1;
            }
            w->alpha -= 10.0f;
            if (w->alpha < 0.0f) {
                return -1;
            }
            w->atk->flags &= ~2;
            break;
        case 3:
            if (w->alpha < 50.0f) {
                w->frame->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->alpha);
            } else {
                w->frame->dl = bossPrimEnvColorDL(255, 255, 0, 255, 255, 255, w->alpha);
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

extern Obj *gBattleTask; /* the battle's task (beffect.c) */
s32 bdevKesaeffectProc(Obj *obj, s16 msg, void *arg);

void kesaEffectCreate(f32 x, f32 y, f32 z, s32 type) {
    KesaEffectArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bdevKesaeffectProc, gBattleTask->file, &arg);
}

/* "Pentani" in the demo (the source's pentaniDemo): "Pentani" 70 times life size with its
 * shadow 70 below, 160 wide; first it lights five Onibi (type 4) 1600 out at the pentagram's
 * points, 400 above it, every 30 frames; at frame 181 it fades in and animates, its rings grow
 * until 50 high, and at frame 1001 it fades out. It ends on gBattleFlags2 8. */
s32 bdevPentaniDemoProc(Obj *obj, s16 msg, void *arg) {
    PentaniW *w = (PentaniW *) FX(obj);
    s32 pad;
    f32 x = 0.0f;
    s32 pad2;
    f32 z = 0.0f;

    switch (msg) {
        case 2:
            obj->name = "Pentani";
            w->shadow = (PolFrame *) frmCreatePrev(gFrmLayer2, gFrmKindPol);
            if (w->shadow == NULL) {
                return -1;
            }
            w->body = (AnimeFrame *) frmCreate(gFrmLayer8, gFrmKindAnime);
            if (w->body == NULL) {
                return -1;
            }
            w->ring[0] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring[0] == NULL) {
                return -1;
            }
            w->ring[1] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring[1] == NULL) {
                return -1;
            }
            bossSetAnimAt(w->body, bdevPentaniBodyAnim, 0xF0, 0.0f);
            bossSetFrameFlags2(w->body, 0x20);
            bossSetPolDL(w->ring[0], (Gfx *) bdevPentaniRingDl, 0xF0);
            fadeAnimStart(&w->anim, pentaniRingAnim, 0, &w->ring[0]->segs[3], floadGetSetFile(0xF0, 0));
            bossSetFrameFlags((AnimeFrame *) w->ring[0], 0x20);
            bossSetPolDL(w->ring[1], (Gfx *) bdevPentaniRingDl, 0xF0);
            fadeAnimStart(&w->anim, pentaniRingAnim, 0, &w->ring[1]->segs[3], floadGetSetFile(0xF0, 0));
            bossSetFrameFlags((AnimeFrame *) w->ring[1], 0x20);
            bossSetPolDL(w->shadow, (Gfx *) bdevPentaDl, 0xF0);
            bossSetFrameFlags((AnimeFrame *) w->shadow, 0x20);
            w->body->scale[0] = w->body->scale[1] = w->body->scale[2] = 70.0f;
            w->body->pos[0] = ((PentaniArg *) arg)->pos[0];
            w->body->pos[1] = ((PentaniArg *) arg)->pos[1];
            w->body->pos[2] = ((PentaniArg *) arg)->pos[2];
            w->ring[0]->scale[0] = w->ring[0]->scale[1] = w->ring[0]->scale[2] = 0.0f;
            w->ring[0]->pos[0] = ((PentaniArg *) arg)->pos[0];
            w->ring[0]->pos[1] = ((PentaniArg *) arg)->pos[1];
            w->ring[0]->pos[2] = ((PentaniArg *) arg)->pos[2];
            w->ring[1]->scale[0] = w->ring[1]->scale[1] = w->ring[1]->scale[2] = 0.0f;
            w->ring[1]->pos[0] = ((PentaniArg *) arg)->pos[0];
            w->ring[1]->pos[1] = ((PentaniArg *) arg)->pos[1];
            w->ring[1]->pos[2] = ((PentaniArg *) arg)->pos[2];
            w->shadow->scale[0] = w->shadow->scale[1] = w->shadow->scale[2] = 160.0f;
            w->shadow->pos[0] = ((PentaniArg *) arg)->pos[0];
            w->shadow->pos[1] = ((PentaniArg *) arg)->pos[1] - 70.0f;
            w->shadow->pos[2] = ((PentaniArg *) arg)->pos[2];
            w->shadow->rot[0] = -1.5707964f;
            w->alpha = 0;
            w->state = 0;
            break;
        case 1:
            frmDestroy(&w->body->f);
            frmDestroy(&w->ring[0]->f);
            frmDestroy(&w->ring[1]->f);
            frmDestroy(&w->shadow->f);
            break;
        case 0:
            if (gBattleFlags2 & 8) {
                return -1;
            }
            switch (w->state) {
                case 0:
                    if (w->time == 0) {
                        bossHeadingOffset(3.141592741f, 0.0f, 1600.0f, &x, &z);
                        x += w->body->pos[0];
                        z += w->body->pos[2];
                        (*onibiCreate)(x, w->body->pos[1] + 400.0f, z, 4);
                    }
                    if (w->time == 30) {
                        bossHeadingOffset(5.654866695f, 0.0f, 1600.0f, &x, &z);
                        x += w->body->pos[0];
                        z += w->body->pos[2];
                        (*onibiCreate)(x, w->body->pos[1] + 400.0f, z, 4);
                    }
                    if (w->time == 60) {
                        bossHeadingOffset(1.884955645f, 0.0f, 1600.0f, &x, &z);
                        x += w->body->pos[0];
                        z += w->body->pos[2];
                        (*onibiCreate)(x, w->body->pos[1] + 400.0f, z, 4);
                    }
                    if (w->time == 90) {
                        bossHeadingOffset(4.398229599f, 0.0f, 1600.0f, &x, &z);
                        x += w->body->pos[0];
                        z += w->body->pos[2];
                        (*onibiCreate)(x, w->body->pos[1] + 400.0f, z, 4);
                    }
                    if (w->time == 120) {
                        bossHeadingOffset(0.6283185482f, 0.0f, 1600.0f, &x, &z);
                        x += w->body->pos[0];
                        z += w->body->pos[2];
                        (*onibiCreate)(x, w->body->pos[1] + 400.0f, z, 4);
                    }
                    if (w->time > 180) {
                        w->state++;
                    }
                    break;
                case 1:
                    w->body->rot[1] += 0.0174532930038800785;
                    bossStepAnim(w->body, 0.3f, 2);
                    w->alpha += 5;
                    if (w->alpha > 255) {
                        w->state++;
                    }
                    break;
                case 2:
                    w->body->rot[1] += 0.0174532930038800785;
                    w->alpha += 5;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                    }
                    if (bossStepAnim(w->body, 0.3f, 2)) {
                        w->state++;
                    }
                    break;
                case 3:
                    fadeAnimStep(&w->anim);
                    w->body->rot[1] += 0.0174532930038800785;
                    w->ring[0]->scale[2] = 8.0f;
                    w->ring[0]->scale[0] = w->ring[0]->scale[2];
                    w->ring[0]->scale[1] += 0.1;
                    w->ring[1]->scale[2] = 8.0f;
                    w->ring[1]->scale[0] = w->ring[1]->scale[2];
                    w->ring[1]->scale[1] += 0.1;
                    w->ring[0]->rot[1] += 0.00872664650194003924;
                    w->ring[1]->rot[1] -= 0.00872664650194003924;
                    if (w->ring[0]->scale[1] > 50.0) {
                        w->state++;
                    }
                    break;
                case 4:
                    w->body->rot[1] += 0.0174532930038800785;
                    fadeAnimStep(&w->anim);
                    w->ring[0]->rot[1] += 0.00872664650194003924;
                    w->ring[1]->rot[1] -= 0.00872664650194003924;
                    if (w->time > 1000) {
                        w->state++;
                    }
                    break;
                case 5:
                    w->body->rot[1] += 0.0174532930038800785;
                    fadeAnimStep(&w->anim);
                    w->ring[0]->rot[1] += 0.00872664650194003924;
                    w->ring[1]->rot[1] -= 0.00872664650194003924;
                    w->alpha -= 5;
                    if (w->alpha <= 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    break;
            }
            w->time++;
            break;
        case 3:
            if (!(gGameFrameCount & 3)) {
                w->body->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->alpha);
            } else {
                w->body->dl = bossPrimEnvColorDL(155, 155, 155, 255, 255, 255, w->alpha);
            }
            w->ring[0]->dl = bossPrimEnvColorDL(255, 255, 100, 255, 0, 0, w->alpha / 3);
            w->ring[1]->dl = bossPrimEnvColorDL(100, 255, 255, 255, 0, 0, w->alpha / 3);
            w->shadow->dl = gBossDepthOnlyDl;
            DPRINTF("[pentaniDemoCreate]タスクの作成に失敗しました。\n");
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The other "Pentani" (bdevPentaniDemoProc) of a type at x, y, z under the effect task. */
void pentaniDemoCreate(f32 x, f32 y, f32 z, s32 type) {
    PentaniArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bdevPentaniDemoProc, gBattleTask->file, &arg);
}

/* "CuBalR" and "CuBalD"'s argument. */
typedef struct CuBalArg {
    /* 0x00 */ u8 pad00[0x20];
    /* 0x20 */ f32 pos[3];
    /* 0x2C */ u8 pad2C[0x4];
    /* 0x30 */ s32 type;
    /* 0x34 */ u8 pad34[0x18];
} CuBalArg;

/* "CuBalR" and "CuBalD"'s work. */
typedef struct CuBalW {
    /* 0x00 */ PolFrame *ball;
    /* 0x04 */ f32 vel[3]; /* bossApplyVel's (bmove.c), in "CuBalD" */
    /* 0x10 */ u8 pad10[8];
    /* 0x18 */ PolFrame *ring[2]; /* cross-fading through CurseBall's four ring pictures */
    /* 0x20 */ u8 pad20[0x10];
    /* 0x30 */ s32 type;
    /* 0x34 */ s32 state;
    /* 0x38 */ s32 time;
    /* 0x3C */ s32 alpha;
    /* 0x40 */ s32 ringAlpha[2];
} CuBalW;

/* "CuBalR"'s work: CuBalD's with a velocity and two boxes. */
typedef struct CuBalRW {
    /* 0x00 */ PolFrame *ball;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ ACollFrame *atk;
    /* 0x14 */ ACollFrame *dmg;
    /* 0x18 */ PolFrame *ring[2];
    /* 0x20 */ u8 pad20[0x10];
    /* 0x30 */ s32 type; /* the ring pictures to start from */
    /* 0x34 */ s32 state;
    /* 0x38 */ s32 time;
    /* 0x3C */ s32 alpha;
    /* 0x40 */ s32 ringAlpha[2];
} CuBalRW;

/* "CuBalR" (the source's curseballR): CurseBall's ball 1.25 times life size at the position
 * given, spinning, with an attack box (radius 30, kind 0x6A) and a damage box (30 by 60, 30
 * down, kind 0x6B), flying at speed 30 at the arena's centre, its two rings cross-fading by 6
 * from the type's pictures on. Landing its attack (a puff, sound 0x248 unless the battle is
 * ending) or hit by kinds 1, 3, 5, 8, 11 or 15 (coins for 1-8, a puff, two showers of pieces,
 * sound 0x248) it stops, both boxes go off and it fades out by 6. */
s32 bdevCuBalRProc(Obj *obj, s16 msg, void *arg) {
    CuBalRW *w = (CuBalRW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CuBalR";
            w->ball = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ball == NULL) {
                return -1;
            }
            w->ring[0] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring[0] == NULL) {
                return -1;
            }
            w->ring[1] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring[1] == NULL) {
                return -1;
            }
            w->type = ((CuBalArg *) arg)->type;
            bossSetPolDL(w->ball, (Gfx *) bdevCurseBallDl, 0xF0);
            bossSetFrameFlags((AnimeFrame *) w->ball, 0x23);
            bossSetPolDL(w->ring[0], (Gfx *) bdevCurseBallRing0Dl, 0xF0);
            bossSetFrameFlags((AnimeFrame *) w->ring[0], 0x27);
            bossSetPolDL(w->ring[1], (Gfx *) bdevCurseBallRing1Dl, 0xF0);
            bossSetFrameFlags((AnimeFrame *) w->ring[1], 0x27);
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->ball->f, 30, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x6A;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->ball->f, 30, 60, -30, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0x6B;
            w->ball->scale[0] = w->ball->scale[1] = w->ball->scale[2] = 1.25f;
            w->ball->pos[0] = ((CuBalArg *) arg)->pos[0];
            w->ball->pos[1] = ((CuBalArg *) arg)->pos[1];
            w->ball->pos[2] = ((CuBalArg *) arg)->pos[2];
            w->ring[0]->scale[0] = w->ring[0]->scale[1] = w->ring[0]->scale[2] = 1.0f;
            w->ring[0]->pos[0] = w->ball->pos[0];
            w->ring[0]->pos[1] = w->ball->pos[1];
            w->ring[0]->pos[2] = w->ball->pos[2];
            w->ring[1]->scale[0] = w->ring[1]->scale[1] = w->ring[1]->scale[2] = 1.0f;
            w->ring[1]->pos[0] = w->ball->pos[0];
            w->ring[1]->pos[1] = w->ball->pos[1];
            w->ring[1]->pos[2] = w->ball->pos[2];
            if (w->type == 0 || w->type == 1 || w->type == 2 || w->type == 3) {
                BmoveX(obj, 0.0f, 0.0f, 0.0f, 30.0f);
            }
            w->time = 0;
            w->alpha = 0;
            w->ringAlpha[0] = 0x80;
            w->ringAlpha[1] = 0;
            if (w->type == 0) {
                w->state = 0;
            }
            if (w->type == 1) {
                w->state = 2;
            }
            if (w->type == 2) {
                w->state = 4;
            }
            if (w->type == 3) {
                w->state = 6;
            } else {
                w->state = 0;
            }
            break;
        case 1:
            frmDestroy(&w->ball->f);
            frmDestroy(&w->ring[0]->f);
            frmDestroy(&w->ring[1]->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
            break;
        case 0:
            bossApplyVel(obj);
            w->ball->rot[2] += 0.0872664650194003855;
            w->ring[0]->pos[0] = w->ball->pos[0];
            w->ring[0]->pos[1] = w->ball->pos[1];
            w->ring[0]->pos[2] = w->ball->pos[2];
            w->ring[1]->pos[0] = w->ball->pos[0];
            w->ring[1]->pos[1] = w->ball->pos[1];
            w->ring[1]->pos[2] = w->ball->pos[2];
            if (acollAtkWasHit(w->atk) == 1) {
                w->atk->flags &= ~2;
                w->dmg->flags &= ~2;
                itexPuff(w->ball->pos[0], w->ball->pos[1], w->ball->pos[2], 0.5f);
                if (!(gBattleFlags & 0x10)) {
                    SoundLocation2(0x248, gBattleCamView, w->ball->pos[0], w->ball->pos[1], w->ball->pos[2], 0.0f,
                                   0.0f, gBattleCamView->fovy);
                }
                w->state = 8;
            }
            if (acollDmgWasHit(w->dmg)
                && (w->dmg->hitKind == 15 || w->dmg->hitKind == 1 || w->dmg->hitKind == 3 || w->dmg->hitKind == 5
                    || w->dmg->hitKind == 8 || w->dmg->hitKind == 11)) {
                if (w->dmg->hitKind == 1 || w->dmg->hitKind == 3 || w->dmg->hitKind == 5 || w->dmg->hitKind == 8) {
                    ryouCreate(w->ball->pos[0], w->ball->pos[1], w->ball->pos[2], 2.0f);
                }
                itexPuff(w->ball->pos[0], w->ball->pos[1], w->ball->pos[2], 0.5f);
                beBpieceCreate(w->ball->pos[0], w->ball->pos[1], w->ball->pos[2], 5.0f, 10);
                beBpieceCreate(w->ball->pos[0], w->ball->pos[1], w->ball->pos[2], 5.0f, 10);
                if (!(gBattleFlags & 0x10)) {
                    SoundLocation2(0x248, gBattleCamView, w->ball->pos[0], w->ball->pos[1], w->ball->pos[2], 300.0f,
                                   15000.0f, gBattleCamView->fovy);
                }
                w->state = 8;
            }
            switch (w->state) {
                case 0:
                    w->alpha += 3;
                    if (w->alpha > 128) {
                        w->alpha = 128;
                    }
                    w->ringAlpha[0] -= 6;
                    if (w->ringAlpha[0] < 0) {
                        w->ringAlpha[0] = 0;
                        bossSetPolDL(w->ring[0], (Gfx *) bdevCurseBallRing1Dl, 0xF0);
                        w->state++;
                    }
                    w->ringAlpha[1] += 6;
                    if (w->ringAlpha[1] > 128) {
                        w->ringAlpha[1] = 128;
                    }
                    break;
                case 1:
                    w->alpha += 3;
                    if (w->alpha > 128) {
                        w->alpha = 128;
                    }
                    w->ringAlpha[0] += 6;
                    if (w->ringAlpha[0] > 128) {
                        w->ringAlpha[0] = 128;
                    }
                    w->ringAlpha[1] -= 6;
                    if (w->ringAlpha[1] < 0) {
                        w->ringAlpha[1] = 0;
                        bossSetPolDL(w->ring[1], (Gfx *) bdevCurseBallRing2Dl, 0xF0);
                        w->state++;
                    }
                    break;
                case 2:
                    w->alpha += 3;
                    if (w->alpha > 128) {
                        w->alpha = 128;
                    }
                    w->ringAlpha[0] -= 6;
                    if (w->ringAlpha[0] < 0) {
                        w->ringAlpha[0] = 0;
                        bossSetPolDL(w->ring[0], (Gfx *) bdevCurseBallRing2Dl, 0xF0);
                        w->state++;
                    }
                    w->ringAlpha[1] += 6;
                    if (w->ringAlpha[1] > 128) {
                        w->ringAlpha[1] = 128;
                    }
                    break;
                case 3:
                    w->alpha += 3;
                    if (w->alpha > 128) {
                        w->alpha = 128;
                    }
                    w->ringAlpha[0] += 6;
                    if (w->ringAlpha[0] > 128) {
                        w->ringAlpha[0] = 128;
                    }
                    w->ringAlpha[1] -= 6;
                    if (w->ringAlpha[1] < 0) {
                        w->ringAlpha[1] = 0;
                        bossSetPolDL(w->ring[1], (Gfx *) bdevCurseBallRing3Dl, 0xF0);
                        w->state++;
                    }
                    break;
                case 4:
                    w->alpha += 3;
                    if (w->alpha > 128) {
                        w->alpha = 128;
                    }
                    w->ringAlpha[0] -= 6;
                    if (w->ringAlpha[0] < 0) {
                        w->ringAlpha[0] = 0;
                        bossSetPolDL(w->ring[0], (Gfx *) bdevCurseBallRing3Dl, 0xF0);
                        w->state++;
                    }
                    w->ringAlpha[1] += 6;
                    if (w->ringAlpha[1] > 128) {
                        w->ringAlpha[1] = 128;
                    }
                    break;
                case 5:
                    w->alpha += 3;
                    if (w->alpha > 128) {
                        w->alpha = 128;
                    }
                    w->ringAlpha[0] += 6;
                    if (w->ringAlpha[0] > 128) {
                        w->ringAlpha[0] = 128;
                    }
                    w->ringAlpha[1] -= 6;
                    if (w->ringAlpha[1] < 0) {
                        w->ringAlpha[1] = 0;
                        bossSetPolDL(w->ring[1], (Gfx *) bdevCurseBallRing0Dl, 0xF0);
                        w->state++;
                    }
                    break;
                case 6:
                    w->ringAlpha[0] -= 6;
                    if (w->ringAlpha[0] < 0) {
                        w->ringAlpha[0] = 0;
                        bossSetPolDL(w->ring[0], (Gfx *) bdevCurseBallRing0Dl, 0xF0);
                        w->state++;
                    }
                    w->ringAlpha[1] += 6;
                    if (w->ringAlpha[1] > 128) {
                        w->ringAlpha[1] = 128;
                    }
                    break;
                case 7:
                    w->alpha += 3;
                    if (w->alpha > 128) {
                        w->alpha = 128;
                    }
                    w->ringAlpha[0] += 6;
                    if (w->ringAlpha[0] > 128) {
                        w->ringAlpha[0] = 128;
                    }
                    w->ringAlpha[1] -= 6;
                    if (w->ringAlpha[1] < 0) {
                        w->ringAlpha[1] = 0;
                        bossSetPolDL(w->ring[1], (Gfx *) bdevCurseBallRing1Dl, 0xF0);
                        w->state = 0;
                    }
                    break;
                case 8:
                    w->alpha -= 6;
                    w->vel[0] = 0.0f;
                    w->vel[1] = 0.0f;
                    w->vel[2] = 0.0f;
                    if (w->alpha < 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    w->ringAlpha[0] -= 6;
                    if (w->ringAlpha[0] < 0) {
                        w->ringAlpha[0] = 0;
                    }
                    w->ringAlpha[1] -= 6;
                    if (w->ringAlpha[1] < 0) {
                        w->ringAlpha[1] = 0;
                    }
                    w->atk->flags &= ~2;
                    w->dmg->flags &= ~2;
                    break;
            }
            w->time++;
            break;
        case 3:
            if (w->time % 9 == 3) {
                w->ball->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->alpha);
            } else {
                w->ball->dl = bossPrimEnvColorDL(200, 200, 255, 255, 255, 255, w->alpha);
            }
            if (w->time % 9 == 6) {
                w->ring[0]->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->ringAlpha[0]);
            } else {
                w->ring[0]->dl = bossPrimEnvColorDL(255, 255, 100, 255, 0, 0, w->ringAlpha[0]);
            }
            if (w->time % 9 == 0) {
                w->ring[1]->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->ringAlpha[1]);
            } else {
                w->ring[1]->dl = bossPrimEnvColorDL(255, 255, 100, 255, 0, 0, w->ringAlpha[1]);
            }
            DPRINTF("[curseballRCreate]タスクの作成に失敗しました。\n");
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "CuBalR" of a type at x, y, z under the effect task. */
void curseballRCreate(f32 x, f32 y, f32 z, s32 type) {
    CuBalArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bdevCuBalRProc, gBattleTask->file, &arg);
}

typedef struct IbulletArg {
    /* 0x00 */ u8 pad00[0x18];
    /* 0x18 */ f32 pos[3];
    /* 0x24 */ u8 pad24[0x4];
    /* 0x28 */ s32 type;
    /* 0x2C */ u8 pad2C[0xC];
} IbulletArg;

/* "Ibullet"'s work. */
typedef struct IbulletW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ ACollFrame *atk;
    /* 0x14 */ ACollFrame *dmg;
    /* 0x18 */ u8 pad18[0xC];
    /* 0x24 */ s32 timer;
    /* 0x28 */ u8 pad28[0x4];
    /* 0x2C */ s32 state; /* 0: flying; 1: knocked away; 2: falling */
    /* 0x30 */ s32 alpha;
    /* 0x34 */ s32 flash; /* set by a vulcan hit: drawn white for a frame */
} IbulletW;

extern u8 bdevIbulletBulletAnim0[]; /* the bullet's animations, in file 0xEA (gBattleFlags 0x4000000 or not) */
extern u8 bdevIbulletBulletAnim1[];
extern u8 bdevIbulletKnockedAwayAnim0[]; /* its knocked-away animations */
extern u8 bdevIbulletKnockedAwayAnim1[];

/* "Ibullet" (the source's impactBullet): an animated bullet from file 0xEA at the position given,
 * six times the size, with an attack box (radius 200, kind 0x71) and a damage box (400 by 300,
 * 150 down, kind 0x72), flying at speed 50 at 100 below the Impact. Landing its attack on
 * anything but kind 13 (the baton), or hit by kinds 1, 3, 5, 8 or 11 (a puff; sound 0x102 unless
 * the battle is ending), both boxes go off and it is knocked away up and back; it spins down,
 * the pilot cries out at frame 15 and it ends after 120 frames. The vulcan (15) only flashes it
 * white. The game's beq on hitKind == 1 to the very next instruction after the damage puff is
 * an empty three-case switch (IDO keeps its first compare, as plonigiri.c's plActNoneDefault).
 * Matched 2026-10-11; it was parked for its compiler temps' stack slots, which the pads around
 * a place. */
s32 bdevIbulletProc(Obj *obj, s16 msg, void *arg) {
    IbulletW *w = (IbulletW *) FX(obj);
    s32 pad;
    f32 a;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "Ibullet";
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer2, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->frame->f, 200, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x71;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 400, 300, -150, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0x72;
            if (gBattleFlags & 0x4000000) {
                bossSetAnimAt(w->frame, bdevIbulletBulletAnim0, 0xEA, 0.0f);
            } else {
                bossSetAnimAt(w->frame, bdevIbulletBulletAnim1, 0xEA, 0.0f);
            }
            bossSetFrameFlags2(w->frame, 0x20);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 6.0f;
            w->frame->pos[0] = ((IbulletArg *) arg)->pos[0];
            w->frame->pos[1] = ((IbulletArg *) arg)->pos[1];
            w->frame->pos[2] = ((IbulletArg *) arg)->pos[2];
            w->frame->rot[0] = 1.5707964f;
            w->alpha = 0x80;
            BmoveX(obj, gImpactPosX, gImpactPosY - 100.0f, gImpactPosZ, 50.0f);
            w->flash = 0;
            w->state = 0;
            break;
        case 0:
            bossApplyVel2(obj);
            bossTurnToward2(w->frame, 0.0f, 0.0f, 5.0f);
            if (acollAtkWasHit(w->atk) == 1) {
                PRINTF("なんかにあたったよATK\n");
                if (w->atk->hitKind == 13) {
                    PRINTF("バトンに当たったけど無視。\n");
                } else {
                    PRINTF("ばぶーんATK\n");
                    w->atk->flags &= ~2;
                    w->dmg->flags &= ~2;
                    itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, bdevIbulletKnockedAwayAnim0, 0xEA, 1.0f);
                    } else {
                        bossSetAnimAt(w->frame, bdevIbulletKnockedAwayAnim1, 0xEA, 1.0f);
                    }
                    a = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                    w->vel[0] = sinf(a) * 10.0f;
                    w->vel[2] = cosf(a) * 10.0f;
                    w->vel[1] = 20.0f;
                    w->state = 1;
                }
            }
            if (acollDmgWasHit(w->dmg)) {
                PRINTF("なんかにあたったよDMG\n");
                if (w->dmg->hitKind == 1 || w->dmg->hitKind == 3 || w->dmg->hitKind == 5 || w->dmg->hitKind == 8
                    || w->dmg->hitKind == 11) {
                    itexPuff(w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 1.0f);
                    switch (w->dmg->hitKind) {
                        case 1:
                            PRINTF("ばぶーんDMG\n");
                            break;
                        case 3:
                            break;
                        case 5:
                            break;
                    }
                    w->atk->flags &= ~2;
                    w->dmg->flags &= ~2;
                    if (gBattleFlags & 0x4000000) {
                        bossSetAnimAt(w->frame, bdevIbulletKnockedAwayAnim0, 0xEA, 1.0f);
                    } else {
                        bossSetAnimAt(w->frame, bdevIbulletKnockedAwayAnim1, 0xEA, 1.0f);
                    }
                    a = mathAtan2(w->frame->pos[0], w->frame->pos[2]);
                    w->vel[0] = sinf(a) * 10.0f;
                    w->vel[2] = cosf(a) * 10.0f;
                    w->vel[1] = 20.0f;
                    if (!(gBattleFlags & 0x10)) {
                        sndPlay(0x102);
                    }
                    w->state = 1;
                } else if (w->dmg->hitKind == 15) {
                    PRINTF("バルカンに当たったけど無視。\n");
                    w->flash = 1;
                }
            }
            switch (w->state) {
                case 0:
                    bossStepAnim(w->frame, 0.2f, 2);
                    break;
                case 1:
                    w->timer = 0;
                    w->state++;
                    break;
                case 2:
                    w->frame->rot[2] += 0.17453293003880077;
                    if (w->timer == 15) {
                        if (gBrotherPilotChara == 0) {
                            sndPlay(0x561);
                        }
                        if (gBrotherPilotChara == 1) {
                            sndPlay(0x560);
                        }
                        if (gBrotherPilotChara == 2) {
                            sndPlay(0x562);
                        }
                        if (gBrotherPilotChara == 3) {
                            sndPlay(0x563);
                        }
                    }
                    if (w->timer++ > 120) {
                        return -1;
                    }
                    break;
            }
            break;
        case 3:
            if (w->flash) {
                w->frame->dl = bossLitTexEnvColorDL(255, 255, 255, 128);
                w->flash = 0;
            } else {
                w->frame->dl = bossLitTexEnvColorDL(0, 0, 0, 0);
            }
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
            PRINTF("[impactBulletCreate]隊長,タスクの作成に失敗しました。\n");
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 bdevIbulletProc(Obj *obj, s16 msg, void *arg);

/* An "Ibullet" of a type at x, y, z under the effect task. */
void impactBulletCreate(f32 x, f32 y, f32 z, s32 type) {
    IbulletArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bdevIbulletProc, gBattleTask->file, &arg);
}

/* "CuBalD": a curse ball from file 0xF0 five times life size at the position given, sinking by
 * 8 a frame to the floor at 0 and pulsing between 4.5 and 5.5, with two rings four times life
 * size that cross-fade by 10 through the four ring pictures in turn. On the floor it raises a
 * splash every 10 frames until frame 150 and clears gBattleFlags2 0x200. At frame 121 it fades
 * out (the ball by 6, the rings by 3) and ends. It ends at once on gBattleFlags2 8. */
s32 bdevCuBalDProc(Obj *obj, s16 msg, void *arg) {
    CuBalW *w = (CuBalW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "CuBalD";
            w->ball = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ball == NULL) {
                return -1;
            }
            w->ring[0] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring[0] == NULL) {
                return -1;
            }
            w->ring[1] = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->ring[1] == NULL) {
                return -1;
            }
            w->type = ((CuBalArg *) arg)->type;
            bossSetPolDL(w->ball, (Gfx *) bdevCurseBallDl, 0xF0);
            bossSetFrameFlags((AnimeFrame *) w->ball, 0x27);
            bossSetPolDL(w->ring[0], (Gfx *) bdevCurseBallRing0Dl, 0xF0);
            bossSetFrameFlags((AnimeFrame *) w->ring[0], 0x27);
            bossSetPolDL(w->ring[1], (Gfx *) bdevCurseBallRing1Dl, 0xF0);
            bossSetFrameFlags((AnimeFrame *) w->ring[1], 0x27);
            w->ball->scale[0] = w->ball->scale[1] = w->ball->scale[2] = 5.0f;
            w->ball->pos[0] = ((CuBalArg *) arg)->pos[0];
            w->ball->pos[1] = ((CuBalArg *) arg)->pos[1];
            w->ball->pos[2] = ((CuBalArg *) arg)->pos[2];
            w->ring[0]->scale[0] = w->ring[0]->scale[1] = w->ring[0]->scale[2] = 4.0f;
            w->ring[0]->pos[0] = w->ball->pos[0];
            w->ring[0]->pos[1] = w->ball->pos[1];
            w->ring[0]->pos[2] = w->ball->pos[2];
            w->ring[1]->scale[0] = w->ring[1]->scale[1] = w->ring[1]->scale[2] = 4.0f;
            w->ring[1]->pos[0] = w->ball->pos[0];
            w->ring[1]->pos[1] = w->ball->pos[1];
            w->ring[1]->pos[2] = w->ball->pos[2];
            w->time = 0;
            w->alpha = 0;
            w->ringAlpha[0] = 0x80;
            w->ringAlpha[1] = 0;
            w->state = 0;
            break;
        case 1:
            frmDestroy(&w->ball->f);
            frmDestroy(&w->ring[0]->f);
            frmDestroy(&w->ring[1]->f);
            break;
        case 0:
            if (gBattleFlags2 & 8) {
                return -1;
            }
            bossApplyVel(obj);
            w->ring[0]->pos[0] = w->ball->pos[0];
            w->ring[0]->pos[1] = w->ball->pos[1];
            w->ring[0]->pos[2] = w->ball->pos[2];
            w->ring[1]->pos[0] = w->ball->pos[0];
            w->ring[1]->pos[1] = w->ball->pos[1];
            w->ring[1]->pos[2] = w->ball->pos[2];
            w->ball->scale[0] = w->ball->scale[1] = w->ball->scale[2] = fadeRandRange(9, 11) / 10.0f * 5.0;
            if (w->ball->pos[1] > 0.0f) {
                w->ball->pos[1] -= 8.0f;
            }
            if (w->ball->pos[1] < 0.0f) {
                w->ball->pos[1] = 0.0f;
            }
            if (w->time > 120) {
                w->state = 8;
            }
            if (w->ball->pos[1] == 0.0f) {
                if (w->time < 151 && w->time % 10 == 0) {
                    beSpeffectCreate(0.0f, -50.0f, -3000.0f, 2);
                }
                gBattleFlags2 &= ~0x200;
            }
            switch (w->state) {
                case 0:
                    w->alpha += 3;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                    }
                    w->ringAlpha[0] -= 10;
                    if (w->ringAlpha[0] < 0) {
                        w->ringAlpha[0] = 0;
                        bossSetPolDL(w->ring[0], (Gfx *) bdevCurseBallRing1Dl, 0xF0);
                        w->state++;
                    }
                    w->ringAlpha[1] += 10;
                    if (w->ringAlpha[1] > 128) {
                        w->ringAlpha[1] = 128;
                    }
                    break;
                case 1:
                    w->alpha += 3;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                    }
                    w->ringAlpha[0] += 10;
                    if (w->ringAlpha[0] > 128) {
                        w->ringAlpha[0] = 128;
                    }
                    w->ringAlpha[1] -= 10;
                    if (w->ringAlpha[1] < 0) {
                        w->ringAlpha[1] = 0;
                        bossSetPolDL(w->ring[1], (Gfx *) bdevCurseBallRing2Dl, 0xF0);
                        w->state++;
                    }
                    break;
                case 2:
                    w->alpha += 3;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                    }
                    w->ringAlpha[0] -= 10;
                    if (w->ringAlpha[0] < 0) {
                        w->ringAlpha[0] = 0;
                        bossSetPolDL(w->ring[0], (Gfx *) bdevCurseBallRing2Dl, 0xF0);
                        w->state++;
                    }
                    w->ringAlpha[1] += 10;
                    if (w->ringAlpha[1] > 128) {
                        w->ringAlpha[1] = 128;
                    }
                    break;
                case 3:
                    w->alpha += 3;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                    }
                    w->ringAlpha[0] += 10;
                    if (w->ringAlpha[0] > 128) {
                        w->ringAlpha[0] = 128;
                    }
                    w->ringAlpha[1] -= 10;
                    if (w->ringAlpha[1] < 0) {
                        w->ringAlpha[1] = 0;
                        bossSetPolDL(w->ring[1], (Gfx *) bdevCurseBallRing3Dl, 0xF0);
                        w->state++;
                    }
                    break;
                case 4:
                    w->alpha += 3;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                    }
                    w->ringAlpha[0] -= 10;
                    if (w->ringAlpha[0] < 0) {
                        w->ringAlpha[0] = 0;
                        bossSetPolDL(w->ring[0], (Gfx *) bdevCurseBallRing3Dl, 0xF0);
                        w->state++;
                    }
                    w->ringAlpha[1] += 10;
                    if (w->ringAlpha[1] > 128) {
                        w->ringAlpha[1] = 128;
                    }
                    break;
                case 5:
                    w->alpha += 3;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                    }
                    w->ringAlpha[0] += 10;
                    if (w->ringAlpha[0] > 128) {
                        w->ringAlpha[0] = 128;
                    }
                    w->ringAlpha[1] -= 10;
                    if (w->ringAlpha[1] < 0) {
                        w->ringAlpha[1] = 0;
                        bossSetPolDL(w->ring[1], (Gfx *) bdevCurseBallRing0Dl, 0xF0);
                        w->state++;
                    }
                    break;
                case 6:
                    w->alpha += 3;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                    }
                    w->ringAlpha[0] -= 10;
                    if (w->ringAlpha[0] < 0) {
                        w->ringAlpha[0] = 0;
                        bossSetPolDL(w->ring[0], (Gfx *) bdevCurseBallRing0Dl, 0xF0);
                        w->state++;
                    }
                    w->ringAlpha[1] += 10;
                    if (w->ringAlpha[1] > 128) {
                        w->ringAlpha[1] = 128;
                    }
                    break;
                case 7:
                    w->alpha += 3;
                    if (w->alpha > 255) {
                        w->alpha = 255;
                    }
                    w->ringAlpha[0] += 10;
                    if (w->ringAlpha[0] > 128) {
                        w->ringAlpha[0] = 128;
                    }
                    w->ringAlpha[1] -= 10;
                    if (w->ringAlpha[1] < 0) {
                        w->ringAlpha[1] = 0;
                        bossSetPolDL(w->ring[1], (Gfx *) bdevCurseBallRing1Dl, 0xF0);
                        w->state = 0;
                    }
                    break;
                case 8:
                    w->alpha -= 6;
                    if (w->alpha < 0) {
                        w->alpha = 0;
                        return -1;
                    }
                    w->ringAlpha[0] -= 3;
                    if (w->ringAlpha[0] < 0) {
                        w->ringAlpha[0] = 0;
                    }
                    w->ringAlpha[1] -= 3;
                    if (w->ringAlpha[1] < 0) {
                        w->ringAlpha[1] = 0;
                    }
                    break;
            }
            w->time++;
            break;
        case 3:
            if (w->time % 9 == 3) {
                w->ball->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->alpha);
            } else {
                w->ball->dl = bossPrimEnvColorDL(200, 200, 255, 255, 255, 255, w->alpha);
            }
            if (w->time % 9 == 6) {
                w->ring[0]->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->ringAlpha[0]);
            } else {
                w->ring[0]->dl = bossPrimEnvColorDL(255, 255, 100, 255, 0, 0, w->ringAlpha[0]);
            }
            if (w->time % 9 == 0) {
                w->ring[1]->dl = bossPrimEnvColorDL(255, 255, 255, 255, 255, 255, w->ringAlpha[1]);
            } else {
                w->ring[1]->dl = bossPrimEnvColorDL(255, 255, 100, 255, 0, 0, w->ringAlpha[1]);
            }
            DPRINTF("[curseballRCreate]タスクの作成に失敗しました。\n");
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* A "CuBalD" of a type at x, y, z under the effect task. */
void bdevCuBalDCreate(f32 x, f32 y, f32 z, s32 type) {
    CuBalArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bdevCuBalDProc, gBattleTask->file, &arg);
}

/* "Bappcam", this boss's entrance: a letterboxed demo. The camera starts 500 up at the arena's
 * centre looking ahead, rises at frame 360, turns to the boss at 900, then cuts between set
 * shots at frames 1150-1241. At 1420 it starts scenario 6 (the boss's introduction); at 1580 it
 * clears gBattleFlags2 0x100000 and gBossFlags 0x10000, then goes back behind the Impact, opens
 * the letterbox and ends at frame 1700, or at once on gBattleFlags2 0x8000 (cleared). Step 8's
 * flags are written BF, BF, BF2, Brother, BF2 (as1 moves the stores).
 * Matched 2026-10-11; it was parked for the slot of message 1's cached frmDestroy pointer
 * (pad[10] puts it at the game's 0x24). */
s32 bdevBappcamProc(Obj *obj, s16 msg, void *arg) {
    BCam *w = BCAM(obj);
    s32 pad[10];

    switch (msg) {
        case 2:
            obj->name = "Bappcam";
            fadeCreate(gEffectTask);
            fadeSetParent(gFramePool10);
            w->cam = (Camera *) tskCreate(obj, dracam, 0, NULL);
            if (w->cam == NULL) {
                return -1;
            }
            camSetFov(w->cam, 0.69813174f);
            camSetSpeed(w->cam, 300.0f);
            camSetEye(w->cam, 0.0f, 500.0f, 0.0f);
            camSetAt(w->cam, 0.0f, 0.0f, -3000.0f);
            camGetView(w->cam);
            gBattleFlags |= 0x10000000;
            gBattleFlags |= 0x1000000;
            gBrotherFlags |= 0x10000;
            fadeSetLetterbox(320, 120, 0, 120);
            w->on = 1;
            w->step = 0;
            w->timer = 0;
            break;
        case 1:
            frmDestroy(w->frame);
            frmDestroy(w->frames[0]);
            frmDestroy(w->frames[1]);
            frmDestroy(w->frames[2]);
            break;
        case 0:
            camGetView(w->cam);
            if (gBattleFlags2 & 0x8000) {
                gBattleFlags2 &= ~0x8000;
                return -1;
            }
            DPRINTF("デモタイマー[%d/%d]\n");
            switch (w->step) {
                case 0:
                    fadeMoveLetterbox(50, 190, 1);
                    if (w->timer == 360) {
                        camMoveFov(w->cam, 0.87266463f);
                        camMoveEye(w->cam, 0.0f, 500.0f, 0.0f);
                        w->step++;
                    }
                    break;
                case 1:
                    fadeMoveLetterbox(50, 190, 1);
                    if (w->timer == 900) {
                        camMoveAt(w->cam, gBossPosX, gBossPosY, gBossPosZ);
                        w->step++;
                    }
                    break;
                case 2:
                    camMoveAt(w->cam, gBossPosX, gBossPosY + 500.0f, gBossPosZ);
                    camMoveEye(w->cam, 2000.0f, gBossPosY + 300.0f, -2000.0f);
                    if (w->timer == 1150) {
                        camSetFov(w->cam, 0.87266463f);
                        camSetEye(w->cam, 580.0f, 920.0f, -2500.0f);
                        camSetAt(w->cam, gBossPosX, gBossPosY + 500.0f, gBossPosZ + 300.0f);
                        w->step++;
                    }
                    break;
                case 3:
                    if (w->timer == 1180) {
                        camSetFov(w->cam, 0.87266463f);
                        camSetEye(w->cam, 1160.0f, 620.0f, -1160.0f);
                        camSetAt(w->cam, gBossPosX, gBossPosY + 200.0f, gBossPosZ);
                        w->step++;
                    }
                    break;
                case 4:
                    camMoveAt(w->cam, gBossPosX, gBossPosY + 1000.0f, gBossPosZ);
                    camMoveEye(w->cam, 2000.0f, gBossPosY + 1000.0f, -2000.0f);
                    if (w->timer == 1210) {
                        camSetFov(w->cam, 0.87266463f);
                        camSetEye(w->cam, -100.0f, 270.0f, -2200.0f);
                        camSetAt(w->cam, gBossPosX, 270.0f, gBossPosZ + 100.0f);
                        w->step++;
                    }
                    break;
                case 5:
                    if (w->timer == 1240) {
                        camSetFov(w->cam, 0.87266463f);
                        camSetEye(w->cam, gBossPosX + 2000.0f, gBossPosY, gBossPosZ - 2000.0f);
                        camSetAt(w->cam, gBossPosX, gBossPosY, gBossPosZ);
                        w->step++;
                    }
                    break;
                case 6:
                    if (w->timer == 1241) {
                        camSetSpeed(w->cam, 120.0f);
                        camMoveEye(w->cam, 1000.0f, gBossPosY - 350.0f, -800.0f);
                        camMoveAt(w->cam, 0.0f, gBossPosY + 100.0f, -2000.0f);
                        w->step++;
                    }
                    break;
                case 7:
                    if (w->timer == 1420) {
                        scRunStart(6, 0, 0);
                    }
                    if (w->timer == 1580) {
                        gBattleFlags2 &= ~0x100000;
                        gBossFlags &= ~0x10000;
                        w->step++;
                    }
                    break;
                case 8:
                    camSetSpeed(w->cam, 10.0f);
                    camSetFov(w->cam, 0.34906587f);
                    camSetEye(w->cam, 0.0f, 0.0f, 0.0f);
                    camSetAt(w->cam, 0.0f, 0.0f, -300.0f);
                    gBattleFlags &= ~0x10000000;
                    gBattleFlags &= ~0x1000000;
                    gBattleFlags2 &= ~0x100;
                    gBrotherFlags &= ~0x10000;
                    gBattleFlags2 |= 0x80000;
                    w->step++;
                    break;
                case 9:
                    fadeOpenLetterbox();
                    camMoveFov(w->cam, 0.87266463f);
                    camMoveEye(w->cam, 0.0f, sinf(gGameFrameCount * 0.0174532930038800785) * 3.0f, 0.0f);
                    camMoveAt(w->cam, gCockpitLookAtX, gCockpitLookAtY, gCockpitLookAtZ);
                    if (w->timer == 1700) {
                        return -1;
                    }
                    break;
            }
            w->timer++;
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* The two "BisuG" tasks' argument: only the position their starters set. */
typedef struct BisuGArg {
    /* 0x00 */ u8 pad00[0x28];
    /* 0x28 */ f32 pos[3];
    /* 0x34 */ u8 pad34[0x2C];
} BisuGArg;

/* The "BisuG"s' work. */
typedef struct BisuGW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ ACollFrame *atk; /* on the Impact's side */
    /* 0x14 */ ACollFrame *dmg;
    /* 0x18 */ Anim anim; /* the first's texture animation */
    /* 0x28 */ u8 pad28[0xC];
    /* 0x34 */ s32 unused34; /* set 0 in msg 2, never read */
    /* 0x38 */ u8 pad38[0x4];
    /* 0x3C */ s32 timer;
    /* 0x40 */ s32 flash; /* set by a vulcan hit */
    /* 0x44 */ s32 cooldown; /* frames until both boxes are live again */
    /* 0x48 */ s32 life;
    /* 0x4C */ s32 dying;
    /* 0x50 */ s32 deadSnd; /* the death sound has played */
    /* 0x54 */ s32 armorDown; /* the Impact is down */
    /* 0x58 */ s32 animSet; /* the death animation is set */
    /* 0x5C */ s32 state;
} BisuGW;

extern u8 bdevBisuG2Anim[]; /* the ship's animations, in file 0xD8 */
extern u8 bdevBisuG2DeathAnim[];

void bdevKissECreate(f32 x, f32 y, f32 z, s32 type);

extern u8 bdevBisuGAnim[]; /* the first ship's animations, in file 0xF4 */
extern u8 bdevBisuGState1Anim[];
extern u8 bdevBisuGState2Anim[];
AnimFrame bisuGAnim[] = {
    { (u32) bisuGTex0, 15 },
    { (u32) bisuGTex1, 15 },
    { 0, 0 },
    { 0, 0 },
}; /* its texture animation, in file 0xF3 */

/* The first "BisuG" (Bismaru's ship): the boss's ship, an animated model from file 0xF4 twice
 * life size, its texture animated from file 0xF3, rising from 1500 below the position given with
 * an attack box on the Impact's side (radius 200, kind 0x73) and a damage box (200 by 500, 300
 * down, kind 0x74), 10 hits of life. It rises turning (a horn at frame 200), then turns to face
 * the Impact and flies at it, then at its chest (blowing kisses, "kissE", as it passes -800),
 * changing its animation each time one ends; the mega cannon (11) and other hits (1, 3, 5, 8)
 * knock it back 150 for 5 frames. Once the Impact is down it backs away, slows and shrinks
 * away; dead, it tumbles to the boss until the Impact's punch (attack kind 0xCC) lands: five
 * fire spirits, a flash, and it ends 25 frames later. */
s32 bdevBisuGProc(Obj *obj, s16 msg, void *arg) {
    BisuGW *w = (BisuGW *) FX(obj);
    s32 pad[2];

    switch (msg) {
        case 2:
            obj->name = "BisuG";
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer2, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(uda_atk_root_frame, &w->frame->f, 200, 0, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x73;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(uda_dmg_root_frame, &w->frame->f, 200, 500, -300, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0x74;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 2.0f;
            bossSetAnimAt(w->frame, bdevBisuGAnim, 0xF4, 0.0f);
            fadeAnimStart(&w->anim, bisuGAnim, 0, &w->frame->segs[3], floadGetSetFile(0xF3, 0));
            bossSetFrameFlags2(w->frame, 0x20);
            w->frame->pos[0] = ((BisuGArg *) arg)->pos[0];
            w->frame->pos[1] = ((BisuGArg *) arg)->pos[1] - 1500.0f;
            w->frame->pos[2] = ((BisuGArg *) arg)->pos[2];
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 2.0f;
            gBossFlags |= 0x20000;
            w->unused34 = 0;
            w->flash = 0;
            w->timer = 0;
            w->cooldown = 0;
            w->life = 10;
            w->state = 0;
            w->dying = 0;
            w->deadSnd = 0;
            w->armorDown = 0;
            break;
        case 1:
            gBossFlags &= ~0x20000;
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
            break;
        case 0:
            w->timer++;
            bossApplyVel2(obj);
            w->cooldown--;
            if (w->cooldown <= 0) {
                fadeAnimStart(&w->anim, bisuGAnim, 0, &w->frame->segs[3], floadGetSetFile(0xF3, 0));
                w->atk->flags |= 2;
                w->dmg->flags |= 2;
                w->cooldown = 0;
            }
            if (w->armorDown == 0 && (gImpactArmor <= 0 || gImpactArmor2 <= 0)) {
                w->armorDown = 1;
                w->state = 3;
            }
            if (w->dying && w->timer > 25) {
                return -1;
            }
            if (w->dying) {
                if (w->timer % 5 == 0) {
                    SoundLocation2(0x52C, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0.0f,
                                   0.0f, gBattleCamView->fovy);
                }
                w->atk->flags &= ~2;
                w->dmg->flags &= ~2;
            }
            if (w->cooldown == 0) {
                acollAtkWasHit(w->atk);
                if (acollDmgWasHit(w->dmg)) {
                    if (w->dmg->hitKind == 11) {
                        PRINTF("めが砲にあたった。\n");
                        w->atk->flags &= ~2;
                        w->dmg->flags &= ~2;
                        if (w->deadSnd == 0) {
                            SoundLocation2(0x27D, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                           w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                        }
                        w->deadSnd = 1;
                        w->state = 6;
                    } else if (w->dmg->hitKind == 1 || w->dmg->hitKind == 3 || w->dmg->hitKind == 5
                               || w->dmg->hitKind == 8) {
                        itexPuff(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 1.0f);
                        switch (w->dmg->hitKind) {
                            case 1:
                                break;
                            case 3:
                                break;
                            case 5:
                                break;
                        }
                        w->atk->flags &= ~2;
                        w->dmg->flags &= ~2;
                        w->life--;
                        if (w->life > 0) {
                            if (w->deadSnd == 0) {
                                SoundLocation2(0x27C, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            } else {
                                PRINTF("なんだおい・死んでるじゃねーか\n");
                            }
                        } else if (w->deadSnd == 0) {
                            SoundLocation2(0x27D, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                           w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                        } else {
                            PRINTF("なんだおい・死んでるじゃねーか\n");
                        }
                        w->state = 7;
                    }
                } else if (w->dmg->hitKind == 15) {
                    w->flash = 1;
                }
            }
            if (w->life <= 0) {
                w->state = 6;
            }
            PRINTF("ビスマルステップ[%d/%d]\n", w->state, w->timer);
            switch (w->state) {
                case 0:
                    w->frame->pos[1] += 5.0f;
                    w->frame->rot[1] -= 0.13962634403104063;
                    bossStepAnim(w->frame, 0.1f, 2);
                    if (w->timer == 200) {
                        SoundLocation2(0x316, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                       0.0f, 0.0f, gBattleCamView->fovy);
                    }
                    if (w->timer > 420) {
                        w->timer = 0;
                        w->state++;
                    }
                    break;
                case 1:
                    if (bossStepAnim(w->frame, 0.1f, 2)) {
                        bossSetAnimAt(w->frame, bdevBisuGState1Anim, 0xF4, 0.0f);
                        fadeAnimStart(&w->anim, bisuGAnim, 0, &w->frame->segs[3],
                                      floadGetSetFile(0xF3, 0));
                    }
                    if (bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f)) {
                        BmoveX(obj, gImpactPosX, gImpactPosY - 200.0f, gImpactPosZ, 5.0f);
                        w->state++;
                    }
                    if (gImpactArmor <= 0 || gImpactArmor2 <= 0) {
                        w->state = 3;
                    }
                    break;
                case 2:
                    w->frame->rot[1] = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
                    if (bossStepAnim(w->frame, 0.4f, 2)) {
                        bossSetAnimAt(w->frame, bdevBisuGState2Anim, 0xF4, 0.0f);
                        fadeAnimStart(&w->anim, bisuGAnim, 0, &w->frame->segs[3],
                                      floadGetSetFile(0xF3, 0));
                    }
                    BmoveX(obj, gImpactPosX, gImpactPosY - 200.0f, -600.0f, 5.0f);
                    if (w->frame->pos[2] > -800.0f) {
                        (*bdevKissECreate)(0.0f, 30.0f, -400.0f, 0);
                        w->frame->pos[2] -= 300.0f;
                    }
                    if (gImpactArmor <= 0 || gImpactArmor2 <= 0) {
                        w->state = 3;
                    }
                    break;
                case 3:
                    bossStepAnim(w->frame, 0.1f, 0);
                    if (bossFlyToward(obj, 0.0f, -100.0f, -3000.0f, 1.0f)) {
                        w->state++;
                    }
                    break;
                case 4:
                    bossDampVel(obj, 0.9f);
                    bossStepAnim(w->frame, 0.1f, 0);
                    w->state++;
                    break;
                case 5:
                    bossDampVel(obj, 0.9f);
                    bossStepAnim(w->frame, 0.1f, 0);
                    w->frame->rot[1] += 0.06981317201552031;
                    w->frame->scale[0] *= 0.97;
                    w->frame->scale[2] *= 0.97;
                    if (w->frame->scale[0] < 0.05) {
                        return -1;
                    }
                    break;
                case 6:
                    w->frame->rot[0] += 0.10471975802328047;
                    w->frame->rot[1] += 0.17453293003880077;
                    w->frame->rot[2] += 0.2443461020543211;
                    BmoveX(obj, gBossPosX, gBossPosY, gBossPosZ, 20.0f);
                    if (acollAtkWasHit(w->atk) == 1 && w->atk->hitKind == 0xCC) {
                        gGameWork.view = gBattleCamView;
                        effectFspiritCreate(w->frame->pos[0], w->frame->pos[1] - 500.0f, w->frame->pos[2], 3.0f, 255,
                                            255, 255, 0, 0, 0);
                        effectFspiritCreate(w->frame->pos[0] + 200.0f, w->frame->pos[1] - 500.0f, w->frame->pos[2],
                                            4.0f, 255, 255, 255, 0, 0, 0);
                        effectFspiritCreate(w->frame->pos[0] + 500.0f, w->frame->pos[1] - 300.0f, w->frame->pos[2],
                                            5.0f, 255, 255, 255, 0, 0, 0);
                        effectFspiritCreate(w->frame->pos[0] - 200.0f, w->frame->pos[1] + 100.0f, w->frame->pos[2],
                                            3.0f, 255, 255, 255, 0, 0, 0);
                        effectFspiritCreate(w->frame->pos[0] - 400.0f, w->frame->pos[1] - 200.0f, w->frame->pos[2],
                                            4.0f, 255, 255, 255, 0, 0, 0);
                        SoundLocation2(0x52C, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                       0.0f, 0.0f, gBattleCamView->fovy);
                        beFlashFCreate(255, 255, 255, 10);
                        w->timer = 0;
                        w->dying = 1;
                    }
                    break;
                case 7:
                    fadeAnimStart(&w->anim, bisuGAnim, 0, &w->frame->segs[3], floadGetSetFile(0xF3, 0));
                    w->frame->pos[2] -= 150.0f;
                    w->cooldown = 5;
                    w->state = 2;
                    break;
                case 8:
                    break;
            }
            break;
        case 3:
            w->frame->dl = bossCombineEnvColorDL(0, 0, 128, 96);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 bdevBisuGProc(Obj *obj, s16 msg, void *arg);

/* A "BisuG" (bdevBisuGProc) at x, y, z under the effect task. */
void bdevBisuGCreate(f32 x, f32 y, f32 z) {
    BisuGArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bdevBisuGProc, gBattleTask->file, &arg);
}

/* The second "BisuG" (Bismaru's ghost ship): the boss's ship, an animated model from file 0xD8
 * six times life size, rising from 1500 below the position given, with an attack box on the
 * Impact's side (radius 100, 100 up, kind 0x73) and a damage box (100 by 150, kind 0x76), 15
 * hits of life. It rises turning, then turns to face the Impact and flies at it, then at the
 * Impact's chest (blowing kisses, "kissE", as it passes -800); the mega cannon (11) knocks it
 * back 150 for 5 frames and ends it, other hits (1, 3, 5, 8) knock it back too. Once the
 * Impact is down it backs away, slows and shrinks away; dead, it sinks under the boss until the
 * Impact's punch (attack kind 0xCC) lands: five fire spirits, a flash, and it ends 25 frames
 * later.
 * Matched 2026-10-11; it was parked for its cached function pointers' stack slots (pad[3] puts
 * them at the game's 0x3C/0x40). */
s32 bdevBisuG2Proc(Obj *obj, s16 msg, void *arg) {
    BisuGW *w = (BisuGW *) FX(obj);
    s32 pad[3];

    switch (msg) {
        case 2:
            obj->name = "BisuG";
            w->frame = (AnimeFrame *) frmCreate(gFrmLayer2, gFrmKindAnime);
            if (w->frame == NULL) {
                return -1;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(uda_atk_root_frame, &w->frame->f, 100, 100, 0, 0);
            if (w->atk == NULL) {
                return -1;
            }
            w->atk->kind = 0x73;
            w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(uda_dmg_root_frame, &w->frame->f, 100, 150, 0, 0, 0);
            if (w->dmg == NULL) {
                return -1;
            }
            w->dmg->kind = 0x76;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 6.0f;
            bossSetAnimAt(w->frame, bdevBisuG2Anim, 0xD8, 0.0f);
            bossSetFrameFlags2(w->frame, 0x20);
            w->frame->dl = gDrawSetupLitTexEdgeDl;
            w->frame->pos[0] = ((BisuGArg *) arg)->pos[0];
            w->frame->pos[1] = ((BisuGArg *) arg)->pos[1] - 1500.0f;
            w->frame->pos[2] = ((BisuGArg *) arg)->pos[2];
            gBossFlags |= 0x20000;
            w->unused34 = 0;
            w->flash = 0;
            w->timer = 0;
            w->cooldown = 0;
            w->life = 15;
            w->state = 0;
            w->dying = 0;
            w->deadSnd = 0;
            w->armorDown = 0;
            w->animSet = 0;
            break;
        case 1:
            gBossFlags &= ~0x20000;
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
            break;
        case 0:
            w->timer++;
            bossApplyVel2(obj);
            bossStepAnim(w->frame, 0.2f, 0);
            w->cooldown--;
            if (w->cooldown <= 0) {
                w->atk->flags |= 2;
                w->dmg->flags |= 2;
                w->cooldown = 0;
            }
            if (w->armorDown == 0 && (gImpactArmor <= 0 || gImpactArmor2 <= 0)) {
                w->armorDown = 1;
                w->state = 3;
            }
            if (w->dying && w->timer > 25) {
                return -1;
            }
            if (w->dying) {
                if (w->timer % 5 == 0) {
                    SoundLocation2(0x52C, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2], 0.0f,
                                   0.0f, gBattleCamView->fovy);
                }
                w->atk->flags &= ~2;
                w->dmg->flags &= ~2;
            }
            if (w->cooldown == 0) {
                acollAtkWasHit(w->atk);
                if (acollDmgWasHit(w->dmg)) {
                    if (w->dmg->hitKind == 11) {
                        PRINTF("めが砲にあたった。\n");
                        w->atk->flags &= ~2;
                        w->dmg->flags &= ~2;
                        if (w->deadSnd == 0) {
                            SoundLocation2(0x27D, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                           w->frame->pos[2], 300.0f, 15000.0f, gBattleCamView->fovy);
                        }
                        w->deadSnd = 1;
                        w->state = 6;
                    } else if (w->dmg->hitKind == 1 || w->dmg->hitKind == 3 || w->dmg->hitKind == 5
                               || w->dmg->hitKind == 8) {
                        itexPuff(w->dmg->hitPos[0], w->dmg->hitPos[1], w->dmg->hitPos[2], 1.0f);
                        switch (w->dmg->hitKind) {
                            case 1:
                                break;
                            case 3:
                                break;
                            case 5:
                                break;
                        }
                        w->atk->flags &= ~2;
                        w->dmg->flags &= ~2;
                        w->life--;
                        if (w->life > 0) {
                            if (w->deadSnd == 0) {
                                SoundLocation2(0x27C, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                               w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                            } else {
                                PRINTF("なんだおい・死んでるじゃねーか\n");
                            }
                        } else if (w->deadSnd == 0) {
                            SoundLocation2(0x27D, gBattleCamView, w->frame->pos[0], w->frame->pos[1],
                                           w->frame->pos[2], 0.0f, 0.0f, gBattleCamView->fovy);
                        } else {
                            PRINTF("なんだおい・死んでるじゃねーか\n");
                        }
                        w->state = 7;
                    }
                } else if (w->dmg->hitKind == 15) {
                    w->flash = 1;
                }
            }
            if (w->animSet == 0) {
                if (w->life <= 0) {
                    bossSetAnimAt(w->frame, bdevBisuG2DeathAnim, 0xD8, 0.0f);
                    w->animSet = 1;
                    w->state = 6;
                }
            } else if (w->life <= 0) {
                w->state = 6;
            }
            PRINTF("ビスマルステップ[%d]\n", w->state);
            switch (w->state) {
                case 0:
                    w->frame->pos[1] += 5.0f;
                    w->frame->rot[1] -= 0.13962634403104063;
                    bossStepAnim(w->frame, 0.1f, 2);
                    if (w->timer > 330) {
                        w->timer = 0;
                        w->state++;
                    }
                    break;
                case 1:
                    bossStepAnim(w->frame, 0.1f, 2);
                    if (bossTurnToward2(w->frame, 0.0f, 0.0f, 4.0f)) {
                        BmoveX(obj, gImpactPosX, gImpactPosY - 200.0f, gImpactPosZ, 5.0f);
                        w->state++;
                    }
                    if (gImpactArmor <= 0 || gImpactArmor2 <= 0) {
                        w->state = 3;
                    }
                    break;
                case 2:
                    w->frame->rot[1] = mathAtan2(-w->frame->pos[0], -w->frame->pos[2]);
                    bossStepAnim(w->frame, 0.4f, 2);
                    BmoveX(obj, gImpactPosX, gImpactPosY - 600.0f, -600.0f, 5.0f);
                    if (w->frame->pos[2] > -800.0f) {
                        (*bdevKissECreate)(0.0f, 0.0f, -400.0f, 0);
                        w->frame->pos[2] -= 300.0f;
                    }
                    if (gImpactArmor <= 0 || gImpactArmor2 <= 0) {
                        w->state = 3;
                    }
                    break;
                case 3:
                    bossStepAnim(w->frame, 0.1f, 0);
                    if (bossFlyToward(obj, 0.0f, -100.0f, -3000.0f, 1.0f)) {
                        w->state++;
                    }
                    break;
                case 4:
                    bossDampVel(obj, 0.9f);
                    bossStepAnim(w->frame, 0.1f, 0);
                    w->state++;
                    break;
                case 5:
                    bossDampVel(obj, 0.9f);
                    bossStepAnim(w->frame, 0.1f, 0);
                    w->frame->rot[1] += 0.06981317201552031;
                    w->frame->scale[0] *= 0.97;
                    w->frame->scale[2] *= 0.97;
                    if (w->frame->scale[0] < 0.05) {
                        return -1;
                    }
                    break;
                case 6:
                    w->frame->rot[1] += 0.34906586007760154;
                    BmoveX(obj, gBossPosX, gBossPosY - 500.0f, gBossPosZ, 20.0f);
                    if (acollAtkWasHit(w->atk) == 1 && w->atk->hitKind == 0xCC) {
                        gGameWork.view = gBattleCamView;
                        effectFspiritCreate(w->frame->pos[0], w->frame->pos[1] - 500.0f, w->frame->pos[2], 3.0f, 255,
                                            255, 255, 0, 0, 0);
                        effectFspiritCreate(w->frame->pos[0] + 200.0f, w->frame->pos[1] - 500.0f, w->frame->pos[2],
                                            4.0f, 255, 255, 255, 0, 0, 0);
                        effectFspiritCreate(w->frame->pos[0] + 500.0f, w->frame->pos[1] - 300.0f, w->frame->pos[2],
                                            5.0f, 255, 255, 255, 0, 0, 0);
                        effectFspiritCreate(w->frame->pos[0] - 200.0f, w->frame->pos[1] + 100.0f, w->frame->pos[2],
                                            3.0f, 255, 255, 255, 0, 0, 0);
                        effectFspiritCreate(w->frame->pos[0] - 400.0f, w->frame->pos[1] - 200.0f, w->frame->pos[2],
                                            4.0f, 255, 255, 255, 0, 0, 0);
                        SoundLocation2(0x52C, gBattleCamView, w->frame->pos[0], w->frame->pos[1], w->frame->pos[2],
                                       0.0f, 0.0f, gBattleCamView->fovy);
                        beFlashFCreate(255, 255, 255, 10);
                        w->timer = 0;
                        w->dying = 1;
                    }
                    break;
                case 7:
                    w->frame->pos[2] -= 150.0f;
                    w->cooldown = 5;
                    w->state = 2;
                    break;
            }
            break;
        case 3:
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 bdevBisuG2Proc(Obj *obj, s16 msg, void *arg);

/* The other "BisuG" (bdevBisuG2Proc) at x, y, z under the effect task. */
void bdevBisuG2Create(f32 x, f32 y, f32 z) {
    BisuGArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bdevBisuG2Proc, gBattleTask->file, &arg);
}

typedef struct HpunchArg {
    /* 0x00 */ u8 pad00[8];
    /* 0x08 */ s32 type;
    /* 0x0C */ u8 pad0C[4];
    /* 0x10 */ f32 pos[3];
    /* 0x1C */ u8 pad1C[4];
} HpunchArg;

/* "kissE"'s work. */
typedef struct KissEW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ ACollFrame *atk;
    /* 0x08 */ u8 pad08[0x4];
    /* 0x0C */ s32 time;
    /* 0x10 */ u8 pad10[0xC];
    /* 0x1C */ f32 alpha;
} KissEW;

extern u8 bdevKissEDl[]; /* the kiss's display list, in file 0xF4 */

/* "kissE" (a blown kiss): tlb_0323_1's second "Hpunch" with its attack box (radius 5000, kind
 * 0x19) in the other side's list (uda_atk_root_frame), turned off once it lands. It grows by
 * 0.002 a frame, shakes up and down by 10 every two frames and fades from 255 by 4, purple to
 * black (sound 0x285 unless the battle is ending). The argument is read through a typed local
 * set in case 2, as tlb_0323_1's first "Hpunch" (bisuHpunchProc) does: that keeps it in v1 for
 * the position loads as the game does, in place of a FAKE empty "if (arg == NULL) {}" (a kept
 * FAKE, 2026-10-17; with pad[2] the local's spill sits a word low, real 2). */
s32 bdevKissEProc(Obj *obj, s16 msg, void *arg) {
    KissEW *w = (KissEW *) FX(obj);
    HpunchArg *a;
    s32 pad;

    switch (msg) {
        case 2:
            obj->name = "kissE";
            w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol);
            if (w->frame == NULL) {
                PRINTF("隊長！モデルを作成できませんでした！！\n");
                return -1;
            }
            w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(uda_atk_root_frame, &w->frame->f, 5000, 0, 0, 0);
            if (w->atk == NULL) {
                PRINTF("隊長！あたりを作成できませんでした！！\n");
                return -1;
            }
            w->atk->kind = 0x19;
            bossSetPolDL(w->frame, (Gfx *) bdevKissEDl, 0xF4);
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2] = 0.12f;
            a = (HpunchArg *) arg;
            w->frame->pos[0] = a->pos[0];
            w->frame->pos[1] = a->pos[1];
            w->frame->pos[2] = a->pos[2];
            bossSetFrameFlags((AnimeFrame *) w->frame, 0x27);
            if (a->type == 0) {
                w->frame->rot[1] = 0.0f;
            } else if (a->type == 1) {
                w->frame->rot[1] = -0.7853982f;
            } else {
                w->frame->rot[1] = 0.7853982f;
            }
            /* one source line, as the game's scheduling has it (as its twin in tlb_0323_1) */
            if (!(gBattleFlags & 0x10)) { sndPlay(0x285); } w->time = 0; w->alpha = 255.0f;
            break;
        case 0:
            if (gBattleFlags & 0x10) {
                return -1;
            }
            if (gBattleFlags2 & 0x400) {
                return -1;
            }
            if (gBattleFlags2 & 0x4000) {
                return -1;
            }
            w->frame->scale[0] += 0.002;
            w->frame->scale[1] += 0.002;
            w->frame->scale[2] += 0.002;
            if (w->time % 4 >= 2) {
                w->frame->pos[1] += 10.0f;
            } else {
                w->frame->pos[1] -= 10.0f;
            }
            w->alpha -= 4.0f;
            if (w->alpha < 0.0f) {
                return -1;
            }
            if (acollAtkWasHit(w->atk) == 1) {
                w->atk->flags &= ~2;
            }
            w->time++;
            break;
        case 3:
            w->frame->dl = bossXluPrimEnvColorDL(w->alpha / 2, 0, w->alpha / 2, w->alpha);
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            PRINTF("隊長,タスクの作成に失敗しました。\n");
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

s32 bdevKissEProc(Obj *obj, s16 msg, void *arg);

void bdevKissECreate(f32 x, f32 y, f32 z, s32 type) {
    HpunchArg arg;

    arg.pos[0] = x;
    arg.pos[1] = y;
    arg.type = type;
    arg.pos[2] = z;
    tskCreate(gEffectTask, bdevKissEProc, gBattleTask->file, &arg);
}
