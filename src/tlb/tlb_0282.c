#include "tlb.h"

extern ConnectData stage1ImanisiPlacesEne0FAParam0[];
extern ConnectData stage1ImanisiPlacesEne0FAParam1[];
extern DoorsData stage1ImanisiPlacesEne127Param2[];
extern DoorsData stage1ImanisiPlacesEne127Param3[];
extern PersonData stage1ImanisiPlacesEne12DParam[];
extern PersonData stage1ImanisiPlacesEne1B9Param[];
extern u16 stage1ImanisiPlacesEne13CParam9Path[];
extern PersonData stage1ImanisiPlacesEne13CParam9[];
extern u16 stage1ImanisiPlacesEne13CParam10Path[];
extern PersonData stage1ImanisiPlacesEne13CParam10[];
ConnectData stage1ImanisiPlacesEne0FAParam0[] = {
    { 15.0f, 25, 0, 746.0f, 0.0f, 111.0f, -0.9424778f, 0x0000, 0, 0 },
};
ConnectData stage1ImanisiPlacesEne0FAParam1[] = {
    { 10.0f, 25, 0, 935.0f, 0.0f, -18.0f, 2.5132742f, 0x0000, 0, 0 },
};
DoorsData stage1ImanisiPlacesEne127Param2[] = {
    { 0.0f, 16 },
};
DoorsData stage1ImanisiPlacesEne127Param3[] = {
    { 0.0f, 17 },
};
PersonData stage1ImanisiPlacesEne12DParam[] = {
    { 477, 0x00000000, { 5, 8 }, 2, 1.5707964f, 0 },
};
PersonData stage1ImanisiPlacesEne1B9Param[] = {
    { 478, 0x00000000, { 5, 8 }, 2, 1.5707964f, 0 },
};
u16 stage1ImanisiPlacesEne13CParam9Path[] = { 0x0000, 0x000C, 0x0064, 0x0002, 0x0000, 0x000C, 0x0024, 0x0002, 0x0001, 0x0000 };
PersonData stage1ImanisiPlacesEne13CParam9[] = {
    { 479, (u32) &stage1ImanisiPlacesEne13CParam9Path, { 5, 8 }, 1, 0.0f, 1 },
};
u16 stage1ImanisiPlacesEne13CParam10Path[] = { 0x0000, 0xFFD8, 0x0024, 0x0004, 0x0000, 0xFFD8, 0x0064, 0x0004, 0x0001, 0x0000 };
PersonData stage1ImanisiPlacesEne13CParam10[] = {
    { 480, (u32) &stage1ImanisiPlacesEne13CParam10Path, { 5, 8 }, 1, 0.0f, 1 },
};
EneDef stage1ImanisiPlaces[] = {
    { 0, -24, 5, 145, ENE_CONNECT, 2, (u32) &stage1ImanisiPlacesEne0FAParam0 },
    { 0, 13, 5, -138, ENE_CONNECT, 2, (u32) &stage1ImanisiPlacesEne0FAParam1 },
    { 0, -30, 0, 125, ENE_DOORS_SET_47, 2, (u32) &stage1ImanisiPlacesEne127Param2 },
    { 0, 10, 0, -122, ENE_DOORS_SET_47, 2, (u32) &stage1ImanisiPlacesEne127Param3 },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, 48, 25, 102, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -68, 15, -23, ENE_SHOP_KYAKU_OYAJI, 2, (u32) &stage1ImanisiPlacesEne12DParam },
    { 0, -68, 15, 10, ENE_SHOP_KYAKU_OBASAN, 2, (u32) &stage1ImanisiPlacesEne1B9Param },
    { 0, 12, 0, 100, ENE_ZISAN, 2, (u32) &stage1ImanisiPlacesEne13CParam9 },
    { 0, -30, 0, 36, ENE_ZISAN, 2, (u32) &stage1ImanisiPlacesEne13CParam10 },
};

void stage1ImanisiInit(void) {
    stageBuildFixedCam(0x29, 0, 0.0f, 0.0f, 0.0f, (u32) stage1ImanisiPlaces, 11, 105.0f, 20.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, 0.0f, 0.7853981853f);
}
