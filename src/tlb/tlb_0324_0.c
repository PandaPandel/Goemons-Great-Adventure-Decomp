#include "tlb.h"
#include "tlb/tlb_0324.h"
s32 btaiEnding = 1; /* the ending: 0 none, 1-3 (btaiEntranceDemo, btaiDeathDemo, btaiWinDemo); the file's .data, padded to 16 */

/* The boss's work (obj + 0x20): tlb_0324_1's BtaiW. */
typedef struct BtaiW {
    /* 0x00 */ AnimeFrame *frame;
    /* 0x04 */ f32 vel[3];
    /* 0x10 */ s32 target[3];
    /* 0x1C */ f32 count;
    /* 0x20 */ PolFrame *drop; /* the model that drops in after the death (tlb_0324_3) */
    /* 0x24 */ ACollFrame *atk;
    /* 0x28 */ ACollFrame *dmg;
    /* 0x2C */ Obj *cam; /* "dracam", the opening camera's (tlb_0324_3) */
    /* 0x30 */ u8 pad30[8];
    /* 0x38 */ CamView *view; /* tlb_0324_3's */
    /* 0x3C */ s32 invTimer; /* steps of invincibility left after a hit (tlb_0324_1) */
    /* 0x40 */ u8 pad40[8];
    /* 0x48 */ f32 speed;
    /* 0x4C */ u8 action;
    /* 0x4D */ u8 endPattern; /* 1: end the pattern now (set as each hit reaction ends) */
    /* 0x4E */ u8 step;
    /* 0x4F */ u8 unused4F;
    /* 0x50 */ s16 timer;
    /* 0x52 */ u8 pattern; /* the row of the phase's patterns (tlb_0324_2) */
    /* 0x53 */ u8 move; /* the action's place in the pattern */
} BtaiW;

typedef ACollFrame *(*AtkAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, s16 oy, s16 oz, s32 a);
typedef ACollFrame *(*DmgAddYZFunc)(ACollFrame *parent, Frame *model, u16 radius, u16 height, s16 oy, s16 oz,
                                    s32 a);

/* tlb_0324_2's: the boss's step (its actions). */
#define btaiStep (*btaiStep)

extern Obj *gBattleTask; /* the battle's task (beffect.c) */
extern s32 btaiNewAction;
extern s32 btaiHitStun;
extern s32 btaiCannonStun;
extern s32 btaiWhirlPiecesEnd;
extern s32 btaiWhirlBubbles;
extern s32 btaiTurned;
extern s32 btaiInkGone;
extern s32 btaiEffectsEnd;
extern f32 btaiWhirlPoint1X;
extern f32 btaiWhirlPoint1Z;
extern s32 gBossLifeMax;
extern s32 gBossFrameCount;
extern u8 btaiAnim[]; /* the boss's animation (file 0xF8) */

#define DPRINTF

/* "Btai", the sea bream boss (the battle's task). Message 2 makes its model of 1.7 with an
 * attack box against the players and a damage box, places it at (0, -600, -2000) and sets the
 * battle up (life 2000, the flags and the counters of every effect); 4 gives its position;
 * 3 draws it yellow while flashing, white for a frame when hit, black while frozen; 0 runs its
 * actions (tlb_0324_2).
 * The game reads msg from its home (`lhu` for the switch, `lh` for the last call): msg is s16
 * and a plain `switch (msg)` gives both (a kept FAKE, 2026-10-17, in place of `*(u16 *) &msg`;
 * `switch ((u16) msg)` is real 82); w is set at its declaration ((689); per case, IDO folds it
 * into obj's offsets in message 1). */
s32 btaiProc(Obj *obj, s16 msg, void *arg) {
    BtaiW *w = FX(obj);
    s32 i;
    s32 pad[11];

    switch (msg) {
        case 2:
            obj->name = "Btai";
            if ((w->frame = (AnimeFrame *) frmCreate(gFrmLayer2, gFrmKindAnime)) == NULL) {
                return -1;
            }
            gBattleTask = obj;
            if ((w->atk = ((AtkAddYZFunc) acollAtkAddYZ)(uda_atk_root_frame, &w->frame->f, 200, 200, 0, 0)) == NULL) {
                return -1;
            }
            w->atk->kind = 0xCD;
            DPRINTF("ATK CREATE\n");
            if ((w->dmg = ((DmgAddYZFunc) acollDmgAddYZ)(ene_dmg_root_frame, &w->frame->f, 230, 550, 0, 0, 0)) == NULL) {
                return -1;
            }
            w->dmg->kind = 0xCC;
            DPRINTF("DMG CREATE\n");
            bossSetAnimAt(w->frame, btaiAnim, 0xF8, 0.0f);
            DPRINTF("ANIME_SET\n");
            w->frame->flags |= 0x20;
            w->frame->scale[2] = 1.7f;
            w->frame->scale[0] = w->frame->scale[1] = w->frame->scale[2];
            w->frame->pos[0] = 0.0f;
            w->frame->pos[1] = -600.0f;
            w->frame->pos[2] = -2000.0f;
            w->action = 0;
            w->timer = 0;
            w->endPattern = 0;
            w->step = 0;
            w->speed = 0.0f;
            gBattleFlags2 |= 0xC00;
            gBossLifeMax = 2000;
            gBossLife = 2000;
            gBossFrameCount = 0;
            gBossPhase = 0;
            gBossActionCount = 0;
            gBossActionArg = 0;
            btaiWhirlBubbles = 0;
            gMegaBurstY = -50.0f;
            btaiEffectsEnd = 0;
            btaiHitStun = 0;
            btaiInkGone = 0;
            btaiWhirlPiecesEnd = 0;
            btaiCannonStun = 0;
            btaiTurned = 0;
            gBossFlags &= ~0x4000;
            gBattleFlags2 |= 0x200000;
            w->pattern = 0;
            w->move = 0;
            btaiEnding = 1;
            btaiNewAction = 1;
            btaiWhirlPoint1X = btaiWhirlPoint1Z = 0.0f;
            for (i = 0; i < 3; i++) {
            }
            fadeCreate(obj);
            fadeSetLetterbox(320, 120, 0, 120);
            DPRINTF("BOSS_INIT_END\n");
            break;
        case 1:
            frmDestroy(&w->frame->f);
            frmDestroy(&w->atk->frame);
            frmDestroy(&w->dmg->frame);
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
                ((BtaiW *) FX(obj))->frame->dl = bossFogEnvColorDL(255, 255, 0, 128, 0, 0, 0, 48, 980);
            } else if (gBossFlags & 0x100) {
                ((BtaiW *) FX(obj))->frame->dl = bossFogEnvColorDL(255, 255, 255, 255, 0, 0, 48, 0, 980);
                gBossFlags &= ~0x100;
            } else if ((gBattleFlags2 & 0x40000) || (gBattleFlags2 & 0x10000)) {
                ((BtaiW *) FX(obj))->frame->dl = bossLitTexEnvColorDL(0, 0, 0, 0);
            } else {
                ((BtaiW *) FX(obj))->frame->dl = bossFogEnvColorDL(0, 0, 0, 0, 0, 0, 48, 0, 980);
            }
            break;
        case 0:
            if (btaiStep(obj) == -1) {
                return -1;
            }
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
