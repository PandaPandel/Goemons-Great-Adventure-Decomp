#include "tlb.h"

extern ConnectData stage1Nagaya3PlacesEne0FAParam[];
extern DoorsData stage1Nagaya3PlacesEne127Param[];
ConnectData stage1Nagaya3PlacesEne0FAParam[] = {
    { 10.0f, 26, 0, -146.0f, 0.0f, 15.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stage1Nagaya3PlacesEne127Param[] = {
    { 0.0f, 19 },
};
EneDef stage1Nagaya3Places[] = {
    { 0, -48, 10, 92, ENE_CONNECT, 2, (u32) &stage1Nagaya3PlacesEne0FAParam },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage1Nagaya3PlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -7, 27, -36, ENE_ROOM_YADOYA_STEAM, 0, 0 },
};

void stage1Nagaya3Init(void) {
    stageBuildFixedCam(0x2C, 0, 0.0f, 10.0f, 0.0f, (u32) stage1Nagaya3Places, 5, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}
