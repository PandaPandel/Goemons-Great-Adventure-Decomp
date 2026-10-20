#include "tlb.h"

extern ConnectData stage1Nagaya5PlacesEne0FAParam[];
extern DoorsData stage1Nagaya5PlacesEne127Param[];
extern PersonData stage1Nagaya5PlacesEne1B9Param[];
ConnectData stage1Nagaya5PlacesEne0FAParam[] = {
    { 10.0f, 26, 0, -21.0f, 0.0f, 16.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stage1Nagaya5PlacesEne127Param[] = {
    { 0.0f, 19 },
};
PersonData stage1Nagaya5PlacesEne1B9Param[] = {
    { 488, 0x00000000, { 5, 8 }, 2, 1.5707964f, 0 },
};
EneDef stage1Nagaya5Places[] = {
    { 0, -48, 10, 92, ENE_CONNECT, 2, (u32) &stage1Nagaya5PlacesEne0FAParam },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage1Nagaya5PlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -7, 27, -36, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -12, 10, -4, ENE_SHOP_KYAKU_OBASAN, 2, (u32) &stage1Nagaya5PlacesEne1B9Param },
};

void stage1Nagaya5Init(void) {
    stageBuildFixedCam(0x2C, 0, 0.0f, 10.0f, 0.0f, (u32) stage1Nagaya5Places, 6, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}
