#include "tlb.h"

extern ConnectData stage1Nagaya1PlacesEne0FAParam[];
extern DoorsData stage1Nagaya1PlacesEne127Param[];
extern u16 stage1Nagaya1PlacesEne13BParamPath[];
extern PersonData stage1Nagaya1PlacesEne13BParam[];
ConnectData stage1Nagaya1PlacesEne0FAParam[] = {
    { 10.0f, 26, 0, -272.0f, 0.0f, 12.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stage1Nagaya1PlacesEne127Param[] = {
    { 0.0f, 19 },
};
u16 stage1Nagaya1PlacesEne13BParamPath[] = { 0x0003, 0x0000, 0x0004, 0x0028, 0x0002, 0x005A, 0x0004, 0x0028, 0x0002, 0x0000, 0x0001, 0x0000 };
PersonData stage1Nagaya1PlacesEne13BParam[] = {
    { 486, (u32) &stage1Nagaya1PlacesEne13BParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef stage1Nagaya1Places[] = {
    { 0, -48, 10, 92, ENE_CONNECT, 2, (u32) &stage1Nagaya1PlacesEne0FAParam },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage1Nagaya1PlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -7, 27, -36, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -31, 0, 31, ENE_ROUNIN, 2, (u32) &stage1Nagaya1PlacesEne13BParam },
};

void stage1Nagaya1Init(void) {
    stageBuildFixedCam(0x2C, 0, 0.0f, 10.0f, 0.0f, (u32) stage1Nagaya1Places, 6, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}
