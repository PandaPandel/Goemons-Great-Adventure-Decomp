#include "tlb.h"

extern ConnectData stage2BannyaPlacesEne0FAParam[];
extern DoorsData stage2BannyaPlacesEne127Param[];
extern PersonData stage2BannyaPlacesEne1A7Param[];
extern PersonData stage2BannyaPlacesEne12EParam[];
ConnectData stage2BannyaPlacesEne0FAParam[] = {
    { 15.0f, 50, 0, -435.0f, 22.0f, 729.0f, 0.31415927f, 0x0000, 0, 0 },
};
DoorsData stage2BannyaPlacesEne127Param[] = {
    { 0.0f, 20 },
};
PersonData stage2BannyaPlacesEne1A7Param[] = {
    { 526, 0x00000000, { 5, 8 }, 2, 1.5707964f, 1 },
};
PersonData stage2BannyaPlacesEne12EParam[] = {
    { 527, 0x00000000, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef stage2BannyaPlaces[] = {
    { 0, 15, 0, 90, ENE_CONNECT, 2, (u32) &stage2BannyaPlacesEne0FAParam },
    { 0, 5, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage2BannyaPlacesEne127Param },
    { 0, -33, 10, 33, ENE_SHOP_BANNYA_OYAZI, 2, (u32) &stage2BannyaPlacesEne1A7Param },
    { 0, 0, 0, 0, ENE_SHOP_YADOYA, 0, 0 },
    { 0, 31, 8, -8, ENE_SHOP_BANNYA_CRIMINAL, 2, (u32) &stage2BannyaPlacesEne12EParam },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -36, 27, 10, ENE_ROOM_YADOYA_STEAM, 0, 0 },
};

void stage2BannyaInit(void) {
    stageBuildFixedCam(0x42, 0, 0.0f, 10.0f, 0.0f, (u32) stage2BannyaPlaces, 8, 85.0f, 20.0f, 50.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 37.0f, 0.5026548505f);
}
