#include "tlb.h"

/* File 112: the townsperson "chounin" (a townsman), a person (person.c) with its animation table
 * in its own file. tlb_0116's twin. */

extern u8 chouninAnim0[];
u32 chouninAnims[] = { (u32) chouninAnim0, 0x03000534, 0x00000000, 0x00000000 }; /* its animations: { 0x030001B8, 0x03000534, 0, 0 } */

s32 chouninProc(Obj *o, s16 msg, void *arg) {
    switch (msg) {
        case 2:
            o->name = "chounin";
            if (personInitMainTable(o, chouninAnims, arg) == -1) {
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
