#include "object.h"
#include "acoll.h"
#include "aniatk.h"
#include "attack.h"
#include "tmanager.h"

/* The attack manager ("Attack", acoll.c's collisions).
 * - Attacks and damage boxes are frames under root frames: an attack root and a damage root,
 *   each with player, enemy and "uda" roots under it.
 * - Each step it updates the roots, then tests player attacks against enemy and uda damage,
 *   enemy attacks against player and uda damage, and uda attacks against all three. */

extern Frame *atk_root_frame; /* the attack root */
extern Frame *pl_atk_root_frame; /* player attacks */
extern Frame *ene_atk_root_frame; /* enemy attacks */
extern Frame *uda_atk_root_frame; /* uda attacks */
extern Frame *dmg_root_frame; /* the damage root */
extern Frame *pl_dmg_root_frame; /* player damage */
extern Frame *ene_dmg_root_frame; /* enemy damage */
extern Frame *uda_dmg_root_frame; /* uda damage */


/* The handler. Made with an optional pair of frame counts. */
s32 attack_Proc(Obj *obj, s16 msg, void *arg) {
    switch (msg) {
        case 2:
            obj->name = "Attack";
            PRINTF("Attack manager をインストール\n");
            if (arg != NULL) {
                acollLinkInitCounts(((s32 *) arg)[0], ((s32 *) arg)[1]);
            } else {
                acollLinkInit();
            }
            if (aniAtkInit() == -1) {
                PRINTF("attack_Proc: acollLinkInit acollAniAtkInitでエラー\n");
                return -1;
            }
            break;
        case 1:
            acollManagerDelete();
            PRINTF("ACOLL: MANAGER 削除\n");
            break;
        case 0:
            acollAtkClearHits(atk_root_frame);
            acollDmgClearHits(dmg_root_frame);
            acollAtkPlaceAll(atk_root_frame);
            acollDmgPlaceAll(dmg_root_frame);
            acollCheckAll(pl_atk_root_frame, ene_dmg_root_frame, 0);
            acollCheckAll(pl_atk_root_frame, uda_dmg_root_frame, 0);
            acollCheckAll(ene_atk_root_frame, pl_dmg_root_frame, 0);
            acollCheckAll(ene_atk_root_frame, uda_dmg_root_frame, 0);
            acollCheckAll(uda_atk_root_frame, pl_dmg_root_frame, 0);
            acollCheckAll(uda_atk_root_frame, ene_dmg_root_frame, 0);
            acollCheckAll(uda_atk_root_frame, uda_dmg_root_frame, 0);
            acollAtkHitsToEnd(pl_atk_root_frame);
            acollAtkHitsToEnd(ene_atk_root_frame);
            acollAtkHitsToEnd(uda_atk_root_frame);
            acollDmgHitsToEnd(pl_dmg_root_frame);
            acollDmgHitsToEnd(ene_dmg_root_frame);
            acollDmgHitsToEnd(uda_dmg_root_frame);
            break;
    }
    return tskDefaultProc(obj, msg, arg);
}
