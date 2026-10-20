#include "tlb.h"

extern ConnectData stage1Nagaya6PlacesEne0FAParam[];
extern DoorsData stage1Nagaya6PlacesEne127Param[];
extern u16 stage1Nagaya6PlacesEne13CParamPath[];
extern PersonData stage1Nagaya6PlacesEne13CParam[];
ConnectData stage1Nagaya6PlacesEne0FAParam[] = {
    { 10.0f, 26, 0, 238.0f, 0.0f, 14.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stage1Nagaya6PlacesEne127Param[] = {
    { 0.0f, 19 },
};
u16 stage1Nagaya6PlacesEne13CParamPath[] = { 0x0000, 0xFFE8, 0xFFD7, 0x0001, 0x0000, 0xFFE8, 0xFFE4, 0x0001, 0x0001, 0x0000 };
PersonData stage1Nagaya6PlacesEne13CParam[] = {
    { 489, (u32) &stage1Nagaya6PlacesEne13CParamPath, { 5, 8 }, 1, 0.0f, 1 },
};
EneDef stage1Nagaya6Places[] = {
    { 0, -48, 10, 92, ENE_CONNECT, 2, (u32) &stage1Nagaya6PlacesEne0FAParam },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage1Nagaya6PlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -7, 27, -36, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -24, 10, -41, ENE_ZISAN, 2, (u32) &stage1Nagaya6PlacesEne13CParam },
};

void stage1Nagaya6Init(void) {
    stageBuildFixedCam(0x2C, 0, 0.0f, 10.0f, 0.0f, (u32) stage1Nagaya6Places, 6, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}
