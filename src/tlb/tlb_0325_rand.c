#include "common.h"
#include "tlb/tlb_0325.h"

/* The battle's two random number generators: their own object (bfriRand starts 16-aligned
 * after tlb_0325_3's padding nops), with their seeds as its .data. */

/* A random number from 0 to n - 1 (a linear congruential generator, 15 bits). Its seed is its
 * static: uopt keeps it in a register and stores it back at the end (IDO_NOTES 316), the store
 * the game makes last. */
s16 bfriRand(s16 n) {
    static u32 seed = 3749696;

    seed = seed * 1103515245 + 5963;
    return (s16) (seed & 0x7FFF) / (0x8000 / n + 1);
}

/* A random number from -n to n - 1, from a second seed. */
s16 bfriRandSigned(s16 n) {
    static u32 seed = 3749696;

    seed = seed * 1103515245 + 5963;
    return (s16) (seed & 0x7FFF) / (0x8000 / (s16) (n * 2) + 1) - n;
}
