#include "tlb.h"

extern ConnectData stage1GoemonPlacesEne0FAParam[];
extern DoorsData stage1GoemonPlacesEne127Param[];
extern PersonData stage1GoemonPlacesEne1DAParam[];
ConnectData stage1GoemonPlacesEne0FAParam[] = {
    { 10.0f, 26, 0, 304.0f, 0.0f, 15.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stage1GoemonPlacesEne127Param[] = {
    { 0.0f, 19 },
};
PersonData stage1GoemonPlacesEne1DAParam[] = {
    { 0, 0x00000000, { 8, 8 }, 2, 1.006292f, 1 },
};
EneDef stage1GoemonPlaces[] = {
    { 0, -48, 0, 92, ENE_CONNECT, 2, (u32) &stage1GoemonPlacesEne0FAParam },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage1GoemonPlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, 15, 28, -1, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -35, 12, 0, ENE_KENSUKE_KENZOU, 2, (u32) &stage1GoemonPlacesEne1DAParam },
    { 0, 0, 0, 0, ENE_KENSUKE_MONEY, 0, 0 },
};

void stage1GoemonInit(void) {
    stageBuildFixedCam(0x2B, 0, 0.0f, 10.0f, 0.0f, (u32) stage1GoemonPlaces, 7, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}
