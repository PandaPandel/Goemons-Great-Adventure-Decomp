#include "tlb.h"

extern ConnectData stage1Nagaya8PlacesEne0FAParam[];
extern DoorsData stage1Nagaya8PlacesEne127Param[];
extern u16 stage1Nagaya8PlacesEne13EParamPath[];
extern PersonData stage1Nagaya8PlacesEne13EParam[];
ConnectData stage1Nagaya8PlacesEne0FAParam[] = {
    { 10.0f, 26, 0, 365.0f, 0.0f, 16.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stage1Nagaya8PlacesEne127Param[] = {
    { 0.0f, 19 },
};
u16 stage1Nagaya8PlacesEne13EParamPath[] = { 0x0009, 0x0005, 0x0000, 0xFFE4, 0x000E, 0x0003, 0x0000, 0xFFE4, 0xFFD9, 0x0003, 0x0000, 0x002F, 0x000C, 0x0003, 0x0001, 0x0000 };
PersonData stage1Nagaya8PlacesEne13EParam[] = {
    { 490, (u32) &stage1Nagaya8PlacesEne13EParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef stage1Nagaya8Places[] = {
    { 0, -48, 10, 92, ENE_CONNECT, 2, (u32) &stage1Nagaya8PlacesEne0FAParam },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage1Nagaya8PlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -7, 27, -36, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -28, 10, 14, ENE_KODOMO, 2, (u32) &stage1Nagaya8PlacesEne13EParam },
};

void stage1Nagaya8Init(void) {
    stageBuildFixedCam(0x2C, 0, 0.0f, 10.0f, 0.0f, (u32) stage1Nagaya8Places, 6, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}
