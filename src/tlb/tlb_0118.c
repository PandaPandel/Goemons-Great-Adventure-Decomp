#include "tlb.h"

/* File 118: the townsperson "kodomo" (a child), a person (person.c) with its animation table
 * in its own file. tlb_0115's twin. */

extern u8 kodomoAnim1[];
u32 kodomoAnims[] = { 0x030003DC, (u32) kodomoAnim1, 0x00000000, 0x00000000 }; /* its animations: { 0x030003DC, 0x03000860, 0, 0 } */

s32 kodomoProc(Obj *o, s16 msg, void *arg) {
    switch (msg) {
        case 2:
            o->name = "kodomo";
            if (personInitMainTable(o, kodomoAnims, arg) == -1) {
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
