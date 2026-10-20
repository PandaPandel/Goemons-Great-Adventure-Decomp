#include "player.h"
#include "aniatk.h"
#include "plbox.h"
#include "plutil.h"

/* The player's attack box (overlay 9's second file). */

/* An action's attack: where its data is in the character's attack file (segment 0xB; plmodel.c
 * loads the file as p->atkBoxPart, from its gPlAtkBoxFileTable list), and the kind plAttackBoxSet gives the
 * box. 33 actions a character, in three runs of 11 (a run ends in a zero entry, but character
 * 2's). Bytes 4-6 are always zero: the game reads the kind with lbu at byte 7. */
typedef struct PlAtkBox {
    /* 0x0 */ u32 data;
    /* 0x4 */ u8 pad4[3];
    /* 0x7 */ u8 kind;
} PlAtkBox; /* size 8 */

PlAtkBox gPlBoxAtkTable[4][33] = {
    {
        { 0x0B000B8C, { 0 }, 0x01 }, { 0x0B000C70, { 0 }, 0x01 }, { 0x0B000E28, { 0 }, 0x01 },
        { 0x0B000FD8, { 0 }, 0x01 }, { 0x0B0010E4, { 0 }, 0x01 }, { 0x0B002080, { 0 }, 0x01 },
        { 0x0B002224, { 0 }, 0x01 }, { 0x0B0023E4, { 0 }, 0x01 }, { 0x0B000190, { 0 }, 0x01 },
        { 0x0B000688, { 0 }, 0x01 }, { 0, { 0 }, 0 },
        { 0x0B001288, { 0 }, 0x02 }, { 0x0B001370, { 0 }, 0x02 }, { 0x0B001528, { 0 }, 0x02 },
        { 0x0B0016DC, { 0 }, 0x02 }, { 0x0B0017D0, { 0 }, 0x02 }, { 0x0B0025AC, { 0 }, 0x02 },
        { 0x0B002754, { 0 }, 0x02 }, { 0x0B002914, { 0 }, 0x02 }, { 0x0B000330, { 0 }, 0x02 },
        { 0x0B000838, { 0 }, 0x02 }, { 0, { 0 }, 0 },
        { 0x0B001978, { 0 }, 0x03 }, { 0x0B001A60, { 0 }, 0x03 }, { 0x0B001C18, { 0 }, 0x03 },
        { 0x0B001DCC, { 0 }, 0x03 }, { 0x0B001EBC, { 0 }, 0x03 }, { 0x0B002ADC, { 0 }, 0x03 },
        { 0x0B002C88, { 0 }, 0x03 }, { 0x0B002E58, { 0 }, 0x03 }, { 0x0B0004DC, { 0 }, 0x03 },
        { 0x0B0009E8, { 0 }, 0x03 }, { 0, { 0 }, 0 },
    },
    {
        { 0x0B000918, { 0 }, 0x12 }, { 0x0B0009EC, { 0 }, 0x12 }, { 0x0B000ACC, { 0 }, 0x12 },
        { 0x0B000C14, { 0 }, 0x12 }, { 0x0B000CDC, { 0 }, 0x12 }, { 0x0B00186C, { 0 }, 0x12 },
        { 0x0B0019BC, { 0 }, 0x12 }, { 0x0B001B10, { 0 }, 0x12 }, { 0x0B000138, { 0 }, 0x12 },
        { 0x0B00052C, { 0 }, 0x12 }, { 0, { 0 }, 0 },
        { 0x0B000E28, { 0 }, 0x13 }, { 0x0B000F00, { 0 }, 0x13 }, { 0x0B000FDC, { 0 }, 0x13 },
        { 0x0B001128, { 0 }, 0x13 }, { 0x0B001200, { 0 }, 0x13 }, { 0x0B001C5C, { 0 }, 0x13 },
        { 0x0B001DA8, { 0 }, 0x13 }, { 0x0B001F00, { 0 }, 0x13 }, { 0x0B000288, { 0 }, 0x13 },
        { 0x0B00067C, { 0 }, 0x13 }, { 0, { 0 }, 0 },
        { 0x0B00134C, { 0 }, 0x14 }, { 0x0B001420, { 0 }, 0x14 }, { 0x0B0014FC, { 0 }, 0x14 },
        { 0x0B001648, { 0 }, 0x14 }, { 0x0B001720, { 0 }, 0x14 }, { 0x0B00204C, { 0 }, 0x14 },
        { 0x0B00219C, { 0 }, 0x14 }, { 0x0B0022F0, { 0 }, 0x14 }, { 0x0B0003D8, { 0 }, 0x14 },
        { 0x0B0007CC, { 0 }, 0x14 }, { 0, { 0 }, 0 },
    },
    {
        { 0x0B000ADC, { 0 }, 0x04 }, { 0x0B000C54, { 0 }, 0x04 }, { 0x0B000E50, { 0 }, 0x01 },
        { 0x0B00103C, { 0 }, 0x01 }, { 0x0B00103C, { 0 }, 0x01 }, { 0x0B001F54, { 0 }, 0x04 },
        { 0x0B002124, { 0 }, 0x01 }, { 0x0B0022B8, { 0 }, 0x04 }, { 0x0B0002F8, { 0 }, 0x04 },
        { 0x0B0007F4, { 0 }, 0x01 }, { 0x0B001AF8, { 0 }, 0x04 },
        { 0x0B00132C, { 0 }, 0x05 }, { 0x0B0014A4, { 0 }, 0x05 }, { 0x0B000E50, { 0 }, 0x02 },
        { 0x0B00103C, { 0 }, 0x02 }, { 0x0B00103C, { 0 }, 0x02 }, { 0x0B00242C, { 0 }, 0x05 },
        { 0x0B002124, { 0 }, 0x02 }, { 0x0B0025A0, { 0 }, 0x05 }, { 0x0B000608, { 0 }, 0x05 },
        { 0x0B0007F4, { 0 }, 0x05 }, { 0x0B001DCC, { 0 }, 0x05 },
        { 0x0B001644, { 0 }, 0x06 }, { 0x0B0017F0, { 0 }, 0x06 }, { 0x0B000E50, { 0 }, 0x03 },
        { 0x0B00103C, { 0 }, 0x03 }, { 0x0B00103C, { 0 }, 0x03 }, { 0x0B00242C, { 0 }, 0x06 },
        { 0x0B002124, { 0 }, 0x03 }, { 0x0B0025A0, { 0 }, 0x06 }, { 0x0B000608, { 0 }, 0x06 },
        { 0x0B0007F4, { 0 }, 0x06 }, { 0x0B001DCC, { 0 }, 0x06 },
    },
    {
        { 0x0B000D2C, { 0 }, 0x04 }, { 0x0B000E2C, { 0 }, 0x04 }, { 0x0B001050, { 0 }, 0x04 },
        { 0x0B001258, { 0 }, 0x04 }, { 0x0B001360, { 0 }, 0x04 }, { 0x0B0025D0, { 0 }, 0x04 },
        { 0x0B0027D0, { 0 }, 0x04 }, { 0x0B0029DC, { 0 }, 0x04 }, { 0x0B0001D4, { 0 }, 0x04 },
        { 0x0B000770, { 0 }, 0x04 }, { 0, { 0 }, 0 },
        { 0x0B00157C, { 0 }, 0x05 }, { 0x0B00167C, { 0 }, 0x05 }, { 0x0B001860, { 0 }, 0x05 },
        { 0x0B001A68, { 0 }, 0x05 }, { 0x0B001B70, { 0 }, 0x05 }, { 0x0B002BE0, { 0 }, 0x05 },
        { 0x0B002DE0, { 0 }, 0x05 }, { 0x0B002FEC, { 0 }, 0x05 }, { 0x0B0003B4, { 0 }, 0x05 },
        { 0x0B000940, { 0 }, 0x05 }, { 0, { 0 }, 0 },
        { 0x0B001D8C, { 0 }, 0x06 }, { 0x0B001E90, { 0 }, 0x06 }, { 0x0B0020B0, { 0 }, 0x06 },
        { 0x0B0022BC, { 0 }, 0x06 }, { 0x0B0023D0, { 0 }, 0x06 }, { 0x0B0031F0, { 0 }, 0x06 },
        { 0x0B0033F0, { 0 }, 0x06 }, { 0x0B0035F8, { 0 }, 0x06 }, { 0x0B000598, { 0 }, 0x06 },
        { 0x0B000B10, { 0 }, 0x06 }, { 0, { 0 }, 0 },
    },
};

/* Per character: the attack file the table above points into (the same numbers as plmodel.c's
 * gPlAtkBoxFileTable list). No code reads this copy; it sits between the table and plswing.c's first
 * label, 16-aligned at both ends, so it is kept with the table it describes. */
u32 gPlBoxAtkFileTable[] = { 0x1A1, 0x1A6, 0x1AB, 0x1B0 };


/* Gives the attack box the kind the character's action n has. */
void plBoxSetKind(Player *p, s32 n) {
    if (p->box != NULL) {
        plAttackBoxSet(p, p->box, gPlBoxAtkTable[p->chara][n].kind);
    }
}

/* Deletes the attack box. */
void plBoxDelete(Player *p) {
    if (p->box != NULL) {
        aniAtkDelete(p->box);
        p->box = NULL;
    }
}

void plBoxAttackBase(Player *p, s32 msg, s32 a, s32 b) {
    switch (msg) {
        case 0:
            plChargeRelease(p);
        case 2:
        default:
            plBaseDefault(p, msg, a, b);
            break;
        case 1:
        case 4:
            break;
    }
}
