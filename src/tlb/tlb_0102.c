#include "tlb.h"
/* transfer's three colours (yellow, magenta, cyan), by its swing count. */
u8 transferColors[][3] = { { 0xFF, 0xFF, 0x10 }, { 0xFF, 0x10, 0xFF }, { 0x10, 0xFF, 0xFF } };
RideData transferRideData[] = {
    { 1, 11.0f, { 0 }, { { 0x0000, { 11.0f, 12.0f, 11.0f }, { 0.0f, -7.0f, 0.0f } } } },
};

#define DPRINTF

/* The swinging task's work (its proc transferProc; transferIdle counts it too). */
typedef struct Tlb102W {
    /* 0x00 */ u8 state;
    /* 0x01 */ u8 angle; /* the spawn's angle when the players come back */
    /* 0x02 */ s16 spawn;
    /* 0x04 */ EneKind *kind;
    /* 0x08 */ u8 flags8;
    /* 0x09 */ u8 unused09;
    /* 0x0A */ u8 flags; /* 1: it splashes */
    /* 0x0B */ u8 unused0B;
    /* 0x0C */ PolFrame *model;
    /* 0x10 */ f32 pos[3]; /* and its place */
    /* 0x1C */ f32 rotX;
    /* 0x20 */ f32 spin;
    /* 0x24 */ f32 rotZ;
    /* 0x28 */ f32 vel[3];
    /* 0x34 */ PolFrame *model2; /* follows the model */
    /* 0x38 */ RideFrame *ride;
    /* 0x3C */ ACollFrame *atk;
    /* 0x40 */ MPolFrame *shadow;
    /* 0x44 */ u32 count;
    /* 0x48 */ f32 baseY; /* the height it bobs about */
    /* 0x4C */ u8 pad4C[0x4];
    /* 0x50 */ f32 splashSize; /* the splashes' growth */
    /* 0x54 */ f32 splashFade;
    /* 0x58 */ f32 squash; /* how hard it squashes the players (transferSquashPlayers) */
    /* 0x5C */ u8 swing; /* 0 to 80 and back */
    /* 0x5D */ s8 swingStep;
    /* 0x5E */ u8 swings; /* times back at 0 */
} Tlb102W;

typedef ACollFrame *(*Tlb102AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);

void transferSwing(Obj *o);
void transferFloat(Obj *o);
void transferSquash(Obj *o);
void transferIdle(Obj *o);
extern Gfx transferDl[]; /* its model */

/* "transfer", the swinging task: a pol model and a second pol (scale 1.0) with an attack box
 * (radius 6, 10 up, kind 0x64). With an angle (the players coming back) it takes flag 2; then it
 * is a ride (file 0x66's data) bobbing about its height, and on the ground it also has flag 1
 * and a shadow, 13 higher. Each frame its state's step runs (the swing, or with flag 2 the
 * float; the squash; the count), it moves, and the second pol follows the model. Drawn in one of
 * three colours by its swing count, its swing the alpha. */
s32 transferProc(Obj *obj, s16 msg, void *arg) {
    Tlb102W *w = (Tlb102W *) FX(obj);

    switch (msg) {
        case 2:
            obj->name = "transfer";
            eneSetup(obj, arg);
            DPRINTF("Ene_transfer size [%d]\n", sizeof(Tlb102W));
            if ((w->model = enePolModelMake(obj, transferDl, NULL, 0)) == NULL) {
                return -1;
            }
            w->model->scale[2] = 1.0f;
            w->model->scale[0] = w->model->scale[1] = w->model->scale[2];
            if ((w->model2 = eneNewPol(obj, NULL)) == NULL) {
                return -1;
            }
            w->model2->scale[2] = 1.0f;
            w->model2->scale[0] = w->model2->scale[1] = w->model2->scale[2];
            if (w->angle != 0) {
                w->flags |= 2;
                w->baseY = w->pos[1];
            }
            if (w->flags & 2) {
                if ((w->ride = rideCreate(gRideTree1, (s32) obj, &w->model2->f,
                                          (RideData *) fileRamAddrAt(0x66, (u32) transferRideData))) == NULL) {
                    DPRINTF("transfer ride NULL\n");
                    return -1;
                }
                if (eneSetOnEarth(obj) == -1) {
                    w->baseY = w->pos[1];
                } else {
                    w->flags |= 1;
                    w->baseY = w->pos[1] + 13.0f;
                    w->shadow = shadowCreate();
                }
            }
            if ((w->atk = ((Tlb102AtkAddYZFunc) acollAtkAddYZ)(ene_atk_root_frame, &w->model->f, 6, 10, 0, 0)) ==
                NULL) {
                return -1;
            }
            w->atk->kind = 0x64;
            w->state = 0;
            w->count = 0;
            break;
        case 1:
            DPRINTF("transfer DESTROY\n");
            if (w->model != NULL) {
                frmDestroy(&w->model->f);
            }
            if (w->model2 != NULL) { frmDestroy(&w->model2->f); }
            if (w->atk != NULL) { frmDestroy(&w->atk->frame); }
            if (w->ride != NULL) { frmDestroy((Frame *) w->ride); }
            if (w->shadow != NULL) { frmDestroy((Frame *) w->shadow); }
            break;
        case 3:
            if (w->model != NULL) {
                w->model->dl = eneDlEnvColorLit(obj, w->swing, transferColors[w->swings % 3][0],
                                                transferColors[w->swings % 3][1],
                                                transferColors[w->swings % 3][2], 1);
            }
            break;
        case 0:
            switch (w->state) {
                case 0:
                    if (w->flags & 2) {
                        transferFloat(obj);
                    } else {
                        transferSwing(obj);
                    }
                    break;
                case 1:
                    transferSquash(obj);
                    break;
                case 2:
                    transferIdle(obj);
                    break;
            }
            eneMove((EneObj *) w);
            if (w->model != NULL) {
                eneSetModelOnCourse(obj, &w->model->f);
                w->model2->pos[0] = w->model->pos[0];
                w->model2->pos[1] = w->model->pos[1];
                w->model2->pos[2] = w->model->pos[2];
            }
            if (w->flags & 2) {
                shadowPut(w->shadow, w->model->pos[0], w->model->pos[1], w->model->pos[2], 1.5f, NULL);
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* Spins by 3 degrees a frame and swings between 0 and 80 by 4. When its attack box is hit hard
 * enough to count, every player in play takes flag 0x40 and it squashes them by a 31st of how
 * far each model is from scale 0 (state 1). The compare is with an integer 0: the game holds
 * the subtraction's 0.0 and the compare's apart (IDO_NOTES 320). */
void transferSwing(Obj *o) {
    Tlb102W *w = (Tlb102W *) FX(o);
    s32 i;
    s32 hitter;

    if (((Tlb102W *) FX(o))->count == 0) {
        w->swingStep = 4;
        w->swing = 0;
    }
    w->count++;
    w->spin += 0.0523598790116402354;
    w->swing += w->swingStep;
    if (w->swing == 80) {
        w->swingStep = -w->swingStep;
    } else if (w->swing == 0) {
        w->swings++;
        w->swingStep = -w->swingStep;
    }
    if (acollAtkWasHit(w->atk) == 1 && (hitter = w->atk->hitPower) != 0) {
        for (i = 0; i < 4; i++) {
            if (plIsInPlay(i)) {
                ((Player *) (gGameWork.players[i] + 1))->flags |= 0x40;
                w->squash = ((0.0 - PL(gGameWork.players[i])->model->scale[0]) / 31.0 > 0)
                                ? (0.0 - PL(gGameWork.players[i])->model->scale[0]) / 31.0
                                : -((0.0 - PL(gGameWork.players[i])->model->scale[0]) / 31.0);
            }
            w->state = 1;
            w->count = 0;
        }
    }
}

/* The floating kind: spins and swings as transferSwing does and bobs 5 about its height;
 * if it splashes, every 8th frame it is below that height it throws a grey splash at the water
 * line (13 under it). A hit from a player in state 5 or 6 squashes every player in play (state
 * 1). */
void transferFloat(Obj *o) {
    Tlb102W *w = (Tlb102W *) FX(o);
    s32 i;
    s32 state;
    s32 hitter;

    if (((Tlb102W *) FX(o))->count == 0) {
        w->swingStep = 4;
        w->swing = 0;
        w->splashSize = 0.012f;
        w->splashFade = 0.004f;
    }
    w->count++;
    w->spin += 0.0523598790116402354;
    w->swing += w->swingStep;
    if (w->swing == 80) {
        w->swingStep = -w->swingStep;
    } else if (w->swing == 0) {
        w->swings++;
        w->swingStep = -w->swingStep;
    }
    w->pos[1] = mathSin(w->spin) * 5.0f + w->baseY;
    if ((w->flags & 1) && !(w->count & 7) && w->pos[1] < w->baseY) {
        effectSibukiCreate(w->model->pos[0], w->baseY - 13.0f, w->model->pos[2], 0.01f, 0.01f,
                           w->splashSize, w->splashSize, w->splashFade, w->splashFade, 0x60, 0x60,
                           0x60, 0xA0, 0xA0, 0xA0);
    }
    if (acollAtkWasHit(w->atk) == 1 && (hitter = w->atk->hitPower) != 0) {
        state = shimPlGetState((Player *) hitter);
        if (state >= 5 && state < 7) {
            for (i = 0; i < 4; i++) {
                if (plIsInPlay(i)) {
                    ((Player *) (gGameWork.players[i] + 1))->flags |= 0x40;
                    w->squash = ((0.0 - PL(gGameWork.players[i])->model->scale[0]) / 31.0 > 0)
                                    ? (0.0 - PL(gGameWork.players[i])->model->scale[0]) / 31.0
                                    : -((0.0 - PL(gGameWork.players[i])->model->scale[0]) / 31.0);
                }
            }
            w->state = 1;
            w->count = 0;
        }
    }
}

void transferHikariCreateAll(Obj *obj);
void transferSquashPlayers(f32 d);

/* The squash (state 1): on its first frame it faces the players, starts their stars
 * (transferHikariCreateAll) and sound 0x160. At frame 15 the screen fades to black with sound
 * 10, and from then on the players are squashed. At 46 it goes to state 2 and moves the players:
 * from any stage to stage 0x8D, keeping this one's place to come back to, and from stage 0x8D
 * back to that place. */
void transferSquash(Obj *o) {
    s32 stage;
    Tlb102W *w;

    if (((Tlb102W *) FX(o))->count == 0) {
        DPRINTF("transfer START\n");
        eneSetForward(o, 1);
        transferHikariCreateAll(o);
        eneSetSd(o, 0x160);
    }
    /* FAKE: a block around the setup (OoT's `if (1) {` spelling; a do-while(0) is the same
     * object); without it IDO keeps o in s0 from the start (IDO_NOTES 321). */
    if (1) {
        w = (Tlb102W *) FX(o);
    }
    if (w->count == 15) {
        DPRINTF("transfer FADE START\n");
        fadeSetAll(30.0f, 0.0f, 255.0f, 0, 0, 0);
        camSunClearTunes();
        gGameWork.sndStopAllFirst = 1;
        gGameWork.tunePart = 0;
        sndPlay(10);
    }
    if (w->count >= 15) {
        transferSquashPlayers(w->squash);
    }
    if (w->count >= 46) {
        DPRINTF("transfer GO\n");
        w->count = 0;
        w->state = 2;
        if (gGameWork.stage != 0x8D) {
            gGameWork.backStage = gGameWork.stage;
            gGameWork.backAngle = w->angle;
            gGameWork.backPos[0] = w->pos[0];
            gGameWork.backPos[1] = w->pos[1];
            gGameWork.backPos[2] = w->pos[2];
            stageGotoSpawnNoSave(gStageTask->parent, 0x8D, 0, 0.0f, 40.0f, 0.0f, 1.5707964f, 0xE0);
        } else {
            stage = gGameWork.backStage;
            stageGotoSpawnNoSave(gStageTask->parent, stage, gGameWork.backAngle,
                                 gGameWork.backPos[0], gGameWork.backPos[1] + 40.0f,
                                 gGameWork.backPos[2], 0.0f, 0xF);
        }
    }
    w->count++;
}

/* Counts the swinging task's frames. */
void transferIdle(Obj *o) {
    Tlb102W *w = (Tlb102W *) FX(o);

    DPRINTF("transfer IDLE [%d]\n", w->count);
    /* FAKE: the empty test keeps w in its own register (addiu v0,a0,0x20; 0x44(v0)) as the
     * game has it; without it IDO folds w into 0x64(a0) (IDO_NOTES 161, 311). */
    if (w != NULL) {
    }
    w->count++;
}

/* "Hikari"'s work (its proc transferHikariProc). */
typedef struct HikariW {
    /* 0x00 */ PolFrame *frame;
    /* 0x04 */ s32 player; /* the one it shines on */
    /* 0x08 */ f32 narrow; /* a frame, from frame 15 */
    /* 0x0C */ f32 grow; /* in height a frame; then how fast it sinks */
    /* 0x10 */ s32 timer;
    /* 0x14 */ u8 state;
} HikariW;

/* Its arg: which player. */
typedef struct HikariArg {
    /* 0x00 */ u8 pad00[0x4];
    /* 0x04 */ s32 player;
} HikariArg;

extern Gfx transferLightDl[]; /* the light */

/* "Hikari", a light on a player: a pol frame on layer 8 (model transferLightDl, file 0x4B) at scale
 * 1.2, turned over, 100 above the player and spinning by 16 degrees a frame. It grows 0.2 a frame
 * in height for 15 frames, narrows by 0.0533 a frame until frame 30, then sinks by 0.2 a frame.
 * Drawn in shade colour. State 3 is a debug mode on pad 2: unless Z is held, the D-pad moves it
 * up and down and widens it, R and L stretch it, C up and down turn it by 5 degrees, and C right
 * and left tip it by 0.01. */
s32 transferHikariProc(Obj *obj, s16 msg, void *arg) {
    s32 pad;
    HikariW *w = (HikariW *) FX(obj);
    f32 x;
    f32 y;
    f32 z;
    s32 pad2;

    switch (msg) {
        case 2:
            obj->name = "Hikari";
            DPRINTF("HIKARI start\n");
            if ((w->frame = (PolFrame *) frmCreate(gFrmLayer8, gFrmKindPol)) == NULL) {
                return -1;
            }
            floadGetSetRam((void **) w->frame->segs, 0x4B);
            w->frame->dl2 = transferLightDl;
            w->frame->flags |= 0x20;
            w->frame->scale[2] = 1.2f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->player = ((HikariArg *) arg)->player;
            plGetModelPos(gGameWork.players[w->player], &x, &y, &z);
            w->frame->pos[0] = x;
            w->frame->pos[1] = y + 100.0f;
            w->frame->pos[2] = z;
            w->frame->rot[0] = 3.1415927f;
            w->narrow = 0.053333335f;
            w->grow = 0.2f;
            w->state = 0;
            break;
        case 1:
            frmDestroy(&w->frame->f);
            break;
        case 0:
            switch (w->state) {
                case 3:
                    if (!(joyGetButton(1, 0) & Z_TRIG)) {
                        if (joyGetButton(1, 0) & U_JPAD) {
                            w->frame->pos[1] -= 1.0f;
                        }
                        if (joyGetButton(1, 0) & D_JPAD) {
                            w->frame->pos[1] += 1.0f;
                        }
                        if (joyGetButton(1, 0) & R_JPAD) {
                            w->frame->scale[2] += 0.01;
                            w->frame->scale[0] = w->frame->scale[2];
                        }
                        if (joyGetButton(1, 0) & L_JPAD) {
                            w->frame->scale[2] -= 0.01;
                            w->frame->scale[0] = w->frame->scale[2];
                        }
                        if (joyGetButton(1, 0) & R_TRIG) {
                            w->frame->scale[1] += 0.01;
                        }
                        if (joyGetButton(1, 0) & L_TRIG) {
                            w->frame->scale[1] -= 0.01;
                        }
                        if (joyGetButton(1, 0) & U_CBUTTONS) {
                            w->frame->rot[1] -= 0.0872664650194003855;
                        }
                        if (joyGetButton(1, 0) & D_CBUTTONS) {
                            w->frame->rot[1] += 0.0872664650194003855;
                        }
                        if (joyGetButton(1, 0) & R_CBUTTONS) {
                            w->frame->rot[0] += 0.01;
                        }
                        if (joyGetButton(1, 0) & L_CBUTTONS) {
                            w->frame->rot[0] -= 0.01;
                        }
                        DPRINTF("sx[%d] sy[%d] sz[%d] ty[%f]\n", (s32) (w->frame->scale[0] * 100.0f),
                                (s32) (w->frame->scale[1] * 100.0f), (s32) (w->frame->scale[2] * 100.0f),
                                w->frame->pos[1]);
                    }
                    break;
                case 0:
                    w->frame->scale[1] += w->grow;
                    if (w->timer == 15) {
                        w->state = 1;
                    }
                    break;
                case 1:
                    w->frame->scale[2] -= w->narrow;
                    w->frame->scale[0] = w->frame->scale[2];
                    if (w->timer == 30) {
                        w->state = 2;
                    }
                    break;
                case 2:
                    w->frame->scale[1] -= w->grow;
                    break;
            }
            plGetModelPos(gGameWork.players[w->player], &x, &y, &z);
            w->frame->pos[0] = x;
            w->frame->pos[1] = y + 100.0f;
            w->frame->pos[2] = z;
            w->frame->rot[1] += 0.279252688062081256;
            w->timer++;
            break;
        case 3:
            w->frame->dl = gGfxDlHead;
            gDPPipeSync(gGfxDlHead++);
            gDPSetCombineMode(gGfxDlHead++, G_CC_SHADE, G_CC_SHADE);
            gSPEndDisplayList(gGfxDlHead++);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}

/* transferHikariProc's argument: only what its starter sets. */
typedef struct Tlb102Arg {
    /* 0x00 */ u8 pad00[0x4];
    /* 0x04 */ s32 player;
    /* 0x08 */ u8 pad08[0x10];
} Tlb102Arg;

s32 transferHikariProc(Obj *obj, s16 msg, void *arg);

/* Starts a transferHikariProc task under the effect task for each player in play whose
 * flags lack 0x10000000. The game reads the flags through PL() in a register (v0): pw's
 * expression holds a load, so IDO keeps pw a variable and does not fold it into the flags read. */
void transferHikariCreateAll(Obj *obj) {
    Tlb102Arg arg;
    s32 pad;
    f32 x;
    f32 y;
    f32 z;
    s32 i;
    Player *pw;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            pw = PL(gGameWork.players[i]);
            if (!(pw->flags & 0x10000000)) {
                plGetModelPos(gGameWork.players[i], &x, &y, &z);
                arg.player = i;
                tskCreate(gEffectTask, transferHikariProc, obj->file, &arg);
            }
        }
    }
}

/* Squashes every player in play (but those with flag 0x10000000): d off the model's z scale,
 * twice d onto its y scale, x the same as z. The game reads the flags through PL(p) in a
 * register (addiu v0,v1,0x20); this C folds them to 0x170(v1) with or without a Player local
 * (21 lines). `d * 2` (an int) keeps the game's mul.s by a loaded 2.0.
 * 2026-10-11: the flags through "^ 0" (transferHikariCreateAll's form) and the model in a local
 * (the game loads it once): 18 lines, mix 0, the same instructions; left: the pointer in t8 for
 * the game's v0 and the float temps (d * 2 in f10, the game's f16). Measured, no change:
 * "scale[1] = scale[1] + d * 2", "2 * d". The permuter's "scale[0] = (scale[2] -= d)" (x the
 * same as z, set first): 6, only the pointer/flags v0-t8 swap left (transferHikariCreateAll's). A named
 * Player pointer with the flags in a local: 6; "^ 0" on p before the add: 9 (folded).
 * 2026-10-11: the flags read inline, no local: 8; flags as u32: 6 (no change).
 * 2026-10-12: "Player *pw = PL(p)" for the flags test and the model, no "^ 0" (no FAKE left):
 * real 2, length -1: the registers are the game's, only its "addiu v0,v1,0x20" is folded into
 * 0x170(v1). The same with "^ 0" on pw: 4 (pw in v1, p in v0; the model through pw). The model
 * as PL(p)->model with pw: 2, the same.
 * 2026-10-12 (second try, from the start form): "pw++; pw--;" after the inner if: 15 (addiu kept but
 * after the flags load, the s registers rotated). That with the model as PL(p)->model: 13
 * (folded). A dead PRINTF("") between pw and the test (IDO_NOTES 2010's lever): 15, the same as
 * ++/--; with the model as PL(p)->model: 13. pltalk.c plTalkSetOthersHalt's matched form (pw and
 * p declared at the top, "pw--;" at the end of the for body, outside plIsInPlay's block): 4, the
 * s registers right, addiu kept but after the flags load (flags 368(p), model 524(pw); the game
 * reads flags 336(pw), model 556(p)); with the model as PL(p)->model: 2 (folded); flags as
 * PL(p)->flags, model pw->model: 4; "pw--" inside plIsInPlay's block: 13; the model read into a
 * local before the test: 3. Then transferHikariCreateAll's match (both FAKEs at once): MATCH. pw =
 * "(Player *) ((p + 0x20) ^ 0)" for the flags test, the model as PL(p)->model, "pw--;" at the
 * end of the for body.
 * 2026-10-17, natural forms: transferHikariCreateAll's
 * match (pw = PL(gGameWork.players[i]), no Obj local, the model as PL(gGameWork.players[i])->model
 * or pw->model): 3, uopt shares the pointer, so the flags fold and the model goes through pw. An
 * Obj local for the model: 4 (v0/v1 swapped); declared in the flags block: 3 (the load in a0, as
 * p's own range takes v1 first). Only two spellings of the one pointer match.
 * 2026-10-17 (certain pass): the OoT/MM catalogs' forms,
 * each with both FAKEs gone: pw as TaskBlock's work member 2; the model through a global reload
 * "((void) 0, gGameWork.players[i])" 3; "pw = PL(p = gGameWork.players[i])" 2; a
 * requiredScopeTemp in the loop 2; an element pointer "&gGameWork.players[i]" 8; a pointer-walk
 * loop 19; FX(p) for PL(p) 2; no pointer local (the flags and the model off PL(..)) 3. */
void transferSquashPlayers(f32 d) {
    s32 i;
    Obj *p;
    Player *pw;

    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            p = gGameWork.players[i];
            /* FAKE: "^ 0" keeps PL(p) in a register as the game has it */
            pw = (Player *) (((u32) p + sizeof(Obj)) ^ 0);
            if (!(pw->flags & 0x10000000)) {
                AnimeFrame *m = PL(p)->model;

                m->scale[0] = (m->scale[2] -= d);
                m->scale[1] += d * 2;
            }
        }
        pw--; /* FAKE: puts pw in v0 as the game has it (a dead decrement; pltalk.c's twin loop, plTalkSetOthersHalt, matches without one in the game's own PL(gGameWork.players[i]) spelling) */
    }
}
