#include "tlb.h"

/* File 113: the townsperson "hikyaku" (a courier), a person (person.c) with its animation table
 * in its own file. tlb_0116's twin. */

extern u8 hikyakuAnim0[];
u32 hikyakuAnims[] = { (u32) hikyakuAnim0, 0x03000AFC, 0x00000000, 0x00000000 }; /* its animations: { 0x03000580, 0x03000AFC, 0, 0 } */

s32 hikyakuProc(Obj *o, s16 msg, void *arg) {
    switch (msg) {
        case 2:
            o->name = "hikyaku";
            if (personInitMainTable(o, hikyakuAnims, arg) == -1) {
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
