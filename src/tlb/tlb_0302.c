#include "tlb.h"

extern ConnectData stage2Minka3PlacesEne0FAParam[];
extern DoorsData stage2Minka3PlacesEne127Param[];
extern u16 stage2Minka3PlacesEne13CParamPath[];
extern PersonData stage2Minka3PlacesEne13CParam[];
ConnectData stage2Minka3PlacesEne0FAParam[] = {
    { 15.0f, 50, 0, -144.0f, 70.0f, -48.0f, 1.8849556f, 0x0000, 0, 0 },
};
DoorsData stage2Minka3PlacesEne127Param[] = {
    { 0.0f, 15 },
};
u16 stage2Minka3PlacesEne13CParamPath[] = { 0x0000, 0xFFD8, 0x002C, 0x0001, 0x0002, 0x00B4, 0x0000, 0xFFD8, 0x0000, 0x0001, 0x0002, 0x0000, 0x0001, 0x0000 };
PersonData stage2Minka3PlacesEne13CParam[] = {
    { 531, (u32) &stage2Minka3PlacesEne13CParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef stage2Minka3Places[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &stage2Minka3PlacesEne0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage2Minka3PlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -40, 11, 44, ENE_ZISAN, 2, (u32) &stage2Minka3PlacesEne13CParam },
};

void stage2Minka3Init(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 10.0f, 0.0f, (u32) stage2Minka3Places, 6, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
}
