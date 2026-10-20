#include "tlb.h"

extern ConnectData stage1IoriPlacesEne0FAParam[];
extern DoorsData stage1IoriPlacesEne127Param[];
extern PersonData stage1IoriPlacesEne1B9Param[];
ConnectData stage1IoriPlacesEne0FAParam[] = {
    { 15.0f, 25, 0, 362.0f, 0.0f, -100.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stage1IoriPlacesEne127Param[] = {
    { 0.0f, 15 },
};
PersonData stage1IoriPlacesEne1B9Param[] = {
    { 475, 0x00000000, { 5, 8 }, 2, 1.5707964f, 0 },
};
EneDef stage1IoriPlaces[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &stage1IoriPlacesEne0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage1IoriPlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -38, 10, 26, ENE_SHOP_KYAKU_OBASAN, 2, (u32) &stage1IoriPlacesEne1B9Param },
};

void stage1IoriInit(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) stage1IoriPlaces, 6, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
}
