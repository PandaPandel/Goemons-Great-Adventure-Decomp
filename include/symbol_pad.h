#ifndef SYMBOL_PAD_H
#define SYMBOL_PAD_H

/* FAKE: BSS REORDER PADDING, the kind F-Zero X keeps (src/overlays/ending/ending.c:53).
 * IDO 7.1 at -g0 lays out a file's .bss by its symbol numbers modulo 256, and of two symbols in
 * one bucket the later comes first (IDO_NOTES 337). Four demo
 * files' last functions show the wrap: WATER (tlb_0019_1), jiji (tlb_0022_1), the old man
 * (tlb_0023_1) and yuki (tlb_0024_1). Their C has ~146 fewer numbers than the game's files had,
 * and what the game spent them on leaves nothing in the ROM: each file needs 143-152 more, and
 * no order of its real declarations reaches 256. An unused static declaration takes a number
 * and emits nothing, so this header, included by those four files ahead of every other
 * declaration, holds 146 of them. They stand for numbers the game spent; no code changes. */

#define SYMBOL_PAD_10(p) \
    static void p##0(void); static void p##1(void); static void p##2(void); \
    static void p##3(void); static void p##4(void); static void p##5(void); \
    static void p##6(void); static void p##7(void); static void p##8(void); \
    static void p##9(void);

SYMBOL_PAD_10(symPadA) SYMBOL_PAD_10(symPadB) SYMBOL_PAD_10(symPadC)
SYMBOL_PAD_10(symPadD) SYMBOL_PAD_10(symPadE) SYMBOL_PAD_10(symPadF)
SYMBOL_PAD_10(symPadG) SYMBOL_PAD_10(symPadH) SYMBOL_PAD_10(symPadI)
SYMBOL_PAD_10(symPadJ) SYMBOL_PAD_10(symPadK) SYMBOL_PAD_10(symPadL)
SYMBOL_PAD_10(symPadM) SYMBOL_PAD_10(symPadN)
static void symPadO0(void); static void symPadO1(void); static void symPadO2(void);
static void symPadO3(void); static void symPadO4(void); static void symPadO5(void);

#endif
