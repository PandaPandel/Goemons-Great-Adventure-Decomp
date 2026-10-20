#include "tlb.h"

extern ConnectData stage2Minka1PlacesEne0FAParam[];
extern DoorsData stage2Minka1PlacesEne127Param[];
extern PersonData stage2Minka1PlacesEne12DParam[];
ConnectData stage2Minka1PlacesEne0FAParam[] = {
    { 15.0f, 50, 0, -168.0f, 57.0f, 180.0f, 1.5707964f, 0x0000, 0, 0 },
};
DoorsData stage2Minka1PlacesEne127Param[] = {
    { 0.0f, 15 },
};
PersonData stage2Minka1PlacesEne12DParam[] = {
    { 530, 0x00000000, { 5, 8 }, 2, 0.62831855f, 1 },
};
EneDef stage2Minka1Places[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &stage2Minka1PlacesEne0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage2Minka1PlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -30, 10, -13, ENE_SHOP_KYAKU_OYAJI, 2, (u32) &stage2Minka1PlacesEne12DParam },
};

void stage2Minka1Init(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 10.0f, 0.0f, (u32) stage2Minka1Places, 6, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
}
