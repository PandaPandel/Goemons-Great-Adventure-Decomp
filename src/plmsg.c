#include "player.h"
#include "game.h"
#include "ombupl.h"
#include "plmsg.h"
#include "plonigiri.h"
#include "plutil.h"

/* Every player action's last default (overlay 9): facing the way it moves, message 3 (hit or
 * given an item, the kind at Player+0x188), 4 (a piggyback request: no), 5 (a demo's next
 * action) and 6 (tags: the chain's "鎖死", Konaki-jiji's "子泣"). */

extern s8 gSavePlayerPips[]; /* per player (10 bytes each): the first row of the HUD's pips */
/* plmsg.c's .data (0x801A84A0-0x801A84B0). */
u16 gPlItem71VoiceTable[4] = { 0x016F, 0x016E, 0x0170, 0x0171 }; /* per character: a voice */



void plBaseDefault(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            break;
        case 1:
            if (fabsf(p->velX) > 0.05f) {
                p->targetFacing = p->facing = (p->velX >= 0.0f) ? 0.0f : 3.1415927f;
            }
            break;
        case 3:
            plHitOrItem(p);
            break;
        case 4:
            plMsgReply(b, 0);
            break;
        case 5:
            if (((s32 *) a)[0] != 0 && ((s32 *) a)[0] == 1) {
                plActSwitch(p, ((s32 *) a)[1]);
                plMsgReply(b, 1);
            }
            break;
        case 6:
            switch (((s32 *) a)[0]) {
                case 0xBBD2B5E3: /* 子泣 */
                    if (!(p->flags & 0x1007AEF0) && p->clinger == NULL) {
                        p->clinger = ((Obj **) a)[1];
                        plActChange(p, 0xC7);
                        plMsgReply((s32) p, 1);
                        return;
                    }
                    plMsgReply((s32) p, 0);
                    return;
                case 0xBABFBBE0: /* 鎖死 */
                    if (p->chara == 0) {
                        p->chain = NULL;
                    }
                    break;
            }
            break;
        default:
            PRINTF("plBase: unknown message\n");
            break;
    }
}

/* Hit, or given an item (message 3). Items: 0x6E-0x70 fill the pips by 1-3, 0x6B money, 0x6C
 * the pips' first row, 0x6D the weapon, 0x74 a life, 0x73 everything. Hits: 0xE, 0x78, 0x7B and
 * 0x7C, 0x7E knock the player back one way or the other. */
void plHitOrItem(Player *p) {
    s32 next = -1;
    s32 flags;

    if (plIsInvincible(p) && p->hitKind >= 0x76) {
        return;
    }
    switch (p->hitKind) {
        case 0x6E:
            plAddHudPips(p, 1);
            break;
        case 0x6F:
            plAddHudPips(p, 2);
            break;
        case 0x70:
            plAddHudPips(p, 3);
            break;
    }
    flags = p->flags;
    if (flags & 0x200) {
        return;
    }
    switch (p->hitKind) {
        case 0x65:
            break;
        case 0x68:
            PRINTF("__getOnigiri(): %d\n");
            p->onigiriTimer = 2;
            break;
        case 0x6A:
            plActChange(p, 9);
            return;
        case 0x6B:
            if (gSaveData.players[p->index].money < 0x8000) {
                gSaveData.players[p->index].money++;
            }
            plSound(p, 0x232);
            return;
        case 0x6C:
            if (gSaveData.players[p->index].life < 3) {
                gSaveData.players[p->index].life++;
                plSound(p, 0x227);
                return;
            }
            plSound(p, 0x233);
            return;
        case 0x6D:
            if (gSaveData.players[p->index].weaponLevel < 2) {
                gSaveData.players[p->index].weaponLevel++;
            }
            plSound(p, 0x233);
            return;
        case 0x72:
            if ((u32) gGameWork.frames >= 5) {
                plSound(p, 0x233);
                return;
            }
            break;
        case 0x71:
            plSound(p, gPlItem71VoiceTable[p->chara]);
            return;
        case 0x74:
            if (p->lifeCooldown == 0) {
                p->lifeCooldown = 5;
                gSaveData.players[p->index].lives++;
                plSound(p, 0x233);
                return;
            }
            break;
        case 0x73:
            gSaveData.players[p->index].life = 3;
            gSaveData.players[p->index].armor = 3;
            gSaveData.players[p->index].weaponLevel = 2;
            plSound(p, 0x233);
            return;
        case 0xE:
        case 0x78:
        case 0x7B:
            p->damage = 1;
            p->weaponDamage = 1;
            if (flags & 0x800) {
                next = 0xB5;
            } else if (flags & 0x400) {
                next = ombuUAction(p, 0xA9);
            } else if (flags & 0x10000) {
                if (p->chara == 2) {
                    next = 0x5A;
                } else {
                    next = 0x4D;
                }
            } else if (flags & 0x20000) {
                if (plDamage(p, p->damage, p->weaponDamage)) {
                    p->flags &= ~0x20004;
                    next = 0xC;
                } else {
                    plVoiceDamage(p);
                    plInvincibleSet(p, 100);
                    plBlinkSet(p, 100);
                }
            } else if (plIsInWater(p)) {
                next = 0x33;
            } else {
                next = 0x30;
            }
            plActChange(p, next);
            return;
        case 0x7C:
        case 0x7E:
            p->damage = -1;
            p->weaponDamage = -1;
            if (flags & 0x800) {
                next = 0xB5;
            } else if (flags & 0x400) {
                next = ombuUAction(p, 0xA9);
            } else if (flags & 0x10000) {
                if (p->chara == 2) {
                    next = 0x5A;
                } else {
                    next = 0x4D;
                }
            } else if (flags & 0x20000) {
                plDamage(p, p->damage, p->weaponDamage);
                next = 0xC;
                p->flags &= ~0x20004;
            } else if (plIsInWater(p)) {
                next = 0x33;
            } else {
                next = 0x30;
            }
            plActChange(p, next);
            return;
        case 0x7D:
            if (plIsOnGround(p)) {
                plActChange(p, 0x2D);
            }
            break;
    }
}

/* Knocked back by the ground (0x1A0 = 2 one way, 3 the other). */
void plGroundKnockback(Player *p) {
    s32 flags;
    s32 next;

    if (!(p->flags & 0x280)) {
        switch (p->ground) {
            case 2:
                p->damage = 1;
                p->weaponDamage = 1;
                break;
            case 3:
                p->damage = -1;
                p->weaponDamage = -1;
                break;
            default:
                return;
        }
        flags = p->flags;
        if (flags & 0x800) {
            next = 0xB5;
        } else if (flags & 0x400) {
            next = ombuUAction(p, 0xA9);
        } else if (flags & 0x10000) {
            if (p->chara == 2) {
                next = 0x5A;
            } else {
                next = 0x4D;
            }
        } else {
            next = 0x30;
        }
        plActChange(p, next);
    }
}

/* Fills the first row of the HUD's pips by n (at most 3), once every 5 steps. */
void plAddHudPips(Player *p, s32 n) {
    s8 v = gSavePlayerPips[p->index * 10];

    if (p->pipsCooldown == 0) {
        if (p->flags & 0x200) {
            plRevive(p);
        }
        if (v < 3) {
            v += n;
            if (v >= 4) {
                v = 3;
            }
            plSound(p, 0x227);
        } else {
            plSound(p, 0x233);
        }
        gSavePlayerPips[p->index * 10] = v;
        p->pipsCooldown = 5;
    }
}
