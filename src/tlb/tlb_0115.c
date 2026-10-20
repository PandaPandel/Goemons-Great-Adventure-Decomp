#include "tlb.h"

/* File 115: the townsperson "rounin" (a masterless samurai), a person (person.c) with its
 * animation table in its own file. */

u32 rouninAnims[] = { 0x03000318, 0x03000758, 0x00000000, 0x00000000 }; /* its animations: { 0x03000318, 0x03000758, 0, 0 } */

s32 rouninProc(Obj *o, s16 msg, void *arg) {
    switch (msg) {
        case 2:
            o->name = "rounin";
            if (personInitMainTable(o, rouninAnims, arg) == -1) {
                return -1;
            }
            break;
        case 8:
            return personTalk(o, arg);
        case 1:
            personDelete(o);
            break;
        case 0:
            personAct(o);
            break;
        case 3: /* nothing: four cases over 0-8 give a compare chain, five the game's jump table
                 * (its words 3-7 all go to tskDefaultProc) */
            break;
    }
    return tskDefaultProc(o, msg, arg);
}
