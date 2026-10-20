#include "game.h"
#include "swwork.h"

/* The event flags ("switch work"), a bit each, that scenario ops 32-35 set, clear and test.
 * - Flags 0-799 are the 100 bytes kept in the game data (gSaveData.flags). Flags 800-807 and
 *   808-815 are two bytes outside it (gSwWork800, gSwWork808), cleared on their own.
 * - A flag past 815 has no byte: the lookup leaves its base unset (a dead warning, below). */

extern u8 gSwWork800[1];   /* flags 800-807 */
extern u8 gSwWork808[100]; /* flags 808-815 (the lookup reaches only the first byte) */

/* Sets, clears and tests a flag. */
void swWorkSet(s32 flag) {
    SwRef r = swWorkCheck(flag);

    r.base[r.index] |= r.mask;
}

void swWorkReset(s32 flag) {
    SwRef r = swWorkCheck(flag);

    r.base[r.index] &= ~r.mask;
}

u8 swWorkTest(s32 flag) {
    SwRef r = swWorkCheck(flag);

    return r.base[r.index] & r.mask;
}

/* Clears the game data's flags. */
void swWorkClearGame(void) {
    s32 i;

    for (i = 0; i < 100; i++) {
        gSaveData.flags[i] = 0;
    }
}

/* Clears the two banks outside the game data. */
void swWorkClear800(void) {
    s32 i;

    for (i = 0; i < 1; i++) {
        gSwWork800[i] = 0;
    }
}

void swWorkClear808(void) {
    s32 i;

    for (i = 0; i < 100; i++) {
        gSwWork808[i] = 0;
    }
}

/* Finds a flag. */
SwRef swWorkCheck(s32 flag) {
    SwRef r;
    u16 byte = flag / 8;
    u16 i = byte;
    u8 *base;
#ifdef AVOID_UB
    base = NULL;
#endif

    if (byte < 100) {
        base = gSaveData.flags;
    } else if (byte < 101) {
        base = gSwWork800;
        i = byte - 100;
    } else if (byte < 102) {
        base = gSwWork808;
        i = byte - 101;
    } else {
        //! @bug base is never set for a flag of 816 or more: after the warning the game returns
        //! whatever was on the stack, and swWorkSet and swWorkReset write through it.
        PRINTF("swWorkCheck: Warning: 利用できる switch ワーク が無いです\n");
    }
    r.index = i;
    r.mask = 1 << (u8) (flag % 8);
    r.base = base;
    return r;
}
