#include "tlb.h"

/* File 119: the townsperson "inu" (a dog), a person (person.c) with its animation table in
 * its own file. tlb_0115's twin. */

extern u8 inuAnim0[];
u32 inuAnims[] = { (u32) inuAnim0, 0x030006C4, 0x00000000, 0x00000000 }; /* its animations: { 0x03000314, 0x030006C4, 0, 0 } */

s32 inuProc(Obj *o, s16 msg, void *arg) {
    switch (msg) {
        case 2:
            o->name = "inu";
            if (personInitMainTable(o, inuAnims, arg) == -1) {
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
