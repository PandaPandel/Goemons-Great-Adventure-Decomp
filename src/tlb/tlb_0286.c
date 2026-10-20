#include "tlb.h"

extern ConnectData stage1BanyaPlacesEne0FAParam[];
extern DoorsData stage1BanyaPlacesEne127Param[];
extern PersonData stage1BanyaPlacesEne1A7Param[];
extern PersonData stage1BanyaPlacesEne12EParam[];
ConnectData stage1BanyaPlacesEne0FAParam[] = {
    { 15.0f, 25, 0, -659.0f, 0.0f, -117.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stage1BanyaPlacesEne127Param[] = {
    { 0.0f, 20 },
};
PersonData stage1BanyaPlacesEne1A7Param[] = {
    { 748, 0x00000000, { 5, 8 }, 2, 1.5707964f, 0 },
};
PersonData stage1BanyaPlacesEne12EParam[] = {
    { 749, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stage1BanyaPlaces[] = {
    { 0, 15, 0, 90, ENE_CONNECT, 2, (u32) &stage1BanyaPlacesEne0FAParam },
    { 0, 5, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage1BanyaPlacesEne127Param },
    { 0, -33, 10, 33, ENE_SHOP_BANNYA_OYAZI, 2, (u32) &stage1BanyaPlacesEne1A7Param },
    { 0, 0, 0, 0, ENE_SHOP_YOROZU_DEBU, 0, 0 },
    { 0, 31, 8, -8, ENE_SHOP_BANNYA_CRIMINAL, 2, (u32) &stage1BanyaPlacesEne12EParam },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -36, 27, 10, ENE_ROOM_YADOYA_STEAM, 0, 0 },
};

void stage1BanyaInit(void) {
    stageBuildFixedCam(0x42, 0, 0.0f, 0.0f, 0.0f, (u32) stage1BanyaPlaces, 8, 85.0f, 20.0f, 50.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 37.0f, 0.5026548505f);
}
