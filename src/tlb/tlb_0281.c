#include "tlb.h"

extern ConnectData stage1MinkaPlacesEne0FAParam[];
extern DoorsData stage1MinkaPlacesEne127Param[];
extern u16 stage1MinkaPlacesEne1BAParamPath[];
extern PersonData stage1MinkaPlacesEne1BAParam[];
ConnectData stage1MinkaPlacesEne0FAParam[] = {
    { 15.0f, 25, 0, 582.0f, 0.0f, 16.0f, -0.31415927f, 0x0000, 0, 0 },
};
DoorsData stage1MinkaPlacesEne127Param[] = {
    { 0.0f, 15 },
};
u16 stage1MinkaPlacesEne1BAParamPath[] = { 0x0004, 0x001E, 0x0002, 0x0000, 0x0004, 0x001E, 0x0002, 0x00B4, 0x0001, 0x0000 };
PersonData stage1MinkaPlacesEne1BAParam[] = {
    { 476, (u32) &stage1MinkaPlacesEne1BAParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stage1MinkaPlaces[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &stage1MinkaPlacesEne0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage1MinkaPlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -26, 10, -10, ENE_SHOP_KYAKU_BOUZU, 2, (u32) &stage1MinkaPlacesEne1BAParam },
};

void stage1MinkaInit(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) stage1MinkaPlaces, 6, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
}
