#include "tlb.h"
#include "tlb/tlb_0325.h"

/* The boss's work (at obj + 0x20): tlb_0325_1's BossW, with what its task sets. */
typedef struct BfriW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ s32 goal[3];
    /* 0x1C */ f32 count;
    /* 0x20 */ ACollFrame *atk;
    /* 0x24 */ ACollFrame *dmg;
    /* 0x28 */ Anim anim;
    /* 0x38 */ Obj *cam; /* "dracam", the opening camera's (tlb_0325_3) */
    /* 0x3C */ u8 pad3C[8];
    /* 0x44 */ CamView *view; /* tlb_0325_3's */
    /* 0x48 */ s32 invTimer; /* steps of invincibility left after a hit (tlb_0325_1) */
    /* 0x4C */ u8 pad4C[0x8];
    /* 0x54 */ f32 speed;
    /* 0x58 */ u8 action;
    /* 0x59 */ u8 endPattern; /* 1: end the pattern now (set as each hit reaction ends) */
    /* 0x5A */ u8 state;
    /* 0x5B */ u8 unused5B;
    /* 0x5C */ s16 time;
    /* 0x5E */ u8 pattern; /* the row of the phase's patterns (tlb_0325_2) */
    /* 0x5F */ u8 move; /* the action's place in the pattern */
} BfriW;

typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

/* tlb_0325_2's: the boss's step (its actions); tlb_0325_7's: starts the bits' parent. */
#define bfriStep (*bfriStep)
#define bfriBitPaCreate (*bfriBitPaCreate)

extern Obj *gBattleTask; /* the battle's task (beffect.c) */
extern AnimFrame bfriTexAnim[]; /* a texture animation (file 0xFA) */
extern s32 bfriSkyShown;
extern s32 bfriNewAction;
extern s32 bfriCloudsGather;
extern s32 bfriCannonStun;
extern s32 bfriCannonHitLast;
extern f32 bfriClearedFloat;
extern f32 bfriClearedFloats;
extern s32 bfriFightOver;
extern s32 bfriCloudsRun; /* 1 while the clouds run */
extern s32 bfriEffectsEnd;
extern s32 gBossLifeMax;
extern s32 gBossFrameCount;
extern u8 bfriAnim[]; /* a model animation (file 0xFB) */

#define DPRINTF

/* "Bfri", the frog boss (the battle's task). Message 2 makes its model of 1.7 with an attack box
 * against the players and a damage box, its texture animation, places it at (0, -200, -2000),
 * sets the battle up (life 3200, the flags and the counters of every effect) and starts the bits;
 * 4 gives its position; 3 draws it yellow while flashing, white for a frame when hit, black while
 * frozen; 0 counts gBossActionArg down, then runs its actions (tlb_0325_2).
 * As tlb_0324_0's Btai, the game reads msg from its home (`sw a1` then `lhu`): msg is s16 and
 * a plain `switch (msg)` gives it (a kept FAKE, 2026-10-17, in place of `*(u16 *) &msg`);
 * `s32 pad[12]` gives the frame. */
s32 bfriProc(Obj *obj, s16 msg, void *arg) {
    BfriW *w = FX(obj);
    s32 pad[12];

    switch (msg) {
        case 2:
            obj->name = "Bfri";
            gBattleTask = obj;
            if ((w->frame = (AnimeFrame *) frmCreate(gFrmLayer2, gFrmKindAnime)) == NULL) {
                return -1;
            }
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(uda_atk_root_frame, &w->frame->f, 290, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0xCD;
            DPRINTF("ATK CREATE\n");
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 240, 500, -250, 0, 0))
                == NULL) {
                return -1;
            }
            w->dmg->kind = 0xCC;
            DPRINTF("DMG CREATE\n");
            bossSetAnimAt(w->frame, bfriAnim, 0xFB, 0.0f);
            fadeAnimStart(&w->anim, bfriTexAnim, 0, &w->frame->segs[3], floadGetSetFile(0xFB, 0));
            DPRINTF("ANIME_SET\n");
            w->frame->flags |= 0x20;
            w->frame->scale[2] = 1.7f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = -200.0f;
            w->frame->pos[2] = -2000.0f;
            w->action = 0;
            w->time = 0;
            w->endPattern = 0;
            w->state = 0;
            w->vel[0] = 0.0f;
            w->vel[1] = 0.0f;
            w->vel[2] = 0.0f;
            w->speed = 0.0f;
            gBattleFlags2 |= 0xC00;
            gBattleFlags2 |= 0x200000;
            gBossLifeMax = 3200;
            gBossLife = 3200;
            gBossFrameCount = 0;
            gBossPhase = 0;
            gBossActionCount = 0;
            gBossActionArg = 0;
            gMegaBurstY = -50.0f;
            bfriEffectsEnd = 0;
            bfriCloudsGather = 0;
            bfriFightOver = 0;
            bfriCannonHitLast = 0;
            bfriCannonStun = 0;
            bfriCloudsRun = 1;
            gBossFlags &= ~0x4000;
            w->invTimer = 0;
            w->pattern = 0;
            w->move = 0;
            bfriSkyShown = 1;
            bfriNewAction = 1;
            bfriClearedFloat = bfriClearedFloats = 0.0f;
            fadeCreate(obj);
            fadeSetLetterbox(320, 120, 0, 120);
            bfriBitPaCreate(obj);
            DPRINTF("BOSS_INIT_END\n");
            break;
        case 1:
            if (w->frame != NULL) {
                frmDestroy(&w->frame->f);
            }
            if (w->atk != NULL) {
                frmDestroy(&w->atk->frame);
            }
            if (w->dmg != NULL) {
                frmDestroy(&w->dmg->frame);
            }
            break;
        case 4:
            if (arg != NULL) {
                ((f32 *) arg)[0] = w->frame->pos[0];
                ((f32 *) arg)[1] = w->frame->pos[1];
                ((f32 *) arg)[2] = w->frame->pos[2];
            }
            break;
        case 3:
            if (gBossFlags & 0x4000) {
                ((BfriW *) FX(obj))->frame->dl = bossFogEnvColorDL(255, 255, 0, 128, 24, 24, 24, 0, 990);
            } else if (gBossFlags & 0x100) {
                ((BfriW *) FX(obj))->frame->dl = bossFogEnvColorDL(255, 255, 255, 255, 24, 24, 24, 0, 990);
                gBossFlags &= ~0x100;
            } else if ((gBattleFlags2 & 0x40000) || (gBattleFlags2 & 0x10000)) {
                ((BfriW *) FX(obj))->frame->dl = bossLitTexEnvColorDL(0, 0, 0, 0);
            } else {
                ((BfriW *) FX(obj))->frame->dl = bossFogEnvColorDL(0, 0, 0, 0, 24, 24, 24, 0, 990);
            }
            break;
        case 0:
            if (gBossActionArg > 0) {
                gBossActionArg--;
                DPRINTF("カウントダウン->%d\n", gBossActionArg);
                break;
            }
            if (bfriStep(obj) == -1) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
