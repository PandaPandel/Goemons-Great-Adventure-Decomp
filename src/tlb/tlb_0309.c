#include "tlb.h"

extern ConnectData stg11Places309Ene0FAParam[];
extern DoorsData stg11Places309Ene127Param[];
extern PersonData stg11Places309Ene12DParam[];
ConnectData stg11Places309Ene0FAParam[] = {
    { 15.0f, 71, 0, 245.0f, 140.0f, -138.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stg11Places309Ene127Param[] = {
    { 0.0f, 15 },
};
PersonData stg11Places309Ene12DParam[] = {
    { 586, 0x00000000, { 5, 8 }, 2, 0.5235988f, 0 },
};
EneDef stg11Places309[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &stg11Places309Ene0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stg11Places309Ene127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -12, 11, 0, ENE_SHOP_KYAKU_OYAJI, 2, (u32) &stg11Places309Ene12DParam },
};

void stg11Build309(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) stg11Places309, 6, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
}
