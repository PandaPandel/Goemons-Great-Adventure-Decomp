#include "tlb.h"

extern ConnectData stage1NoukaPlacesEne0FAParam[];
extern DoorsData stage1NoukaPlacesEne127Param[];
extern u16 stage1NoukaPlacesEne1BBParamPath[];
extern PersonData stage1NoukaPlacesEne1BBParam[];
extern PersonData stage1NoukaPlacesEne1B9Param[];
ConnectData stage1NoukaPlacesEne0FAParam[] = {
    { 15.0f, 25, 0, 1340.0f, -19.0f, 368.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stage1NoukaPlacesEne127Param[] = {
    { 0.0f, 18 },
};
u16 stage1NoukaPlacesEne1BBParamPath[] = { 0x0000, 0x005A, 0x0014, 0x0002, 0x0002, 0x005A, 0x0004, 0x003C, 0x0000, 0x0049, 0xFFAE, 0x0003, 0x0004, 0x001E, 0x0001, 0x0000 };
PersonData stage1NoukaPlacesEne1BBParam[] = {
    { 484, (u32) &stage1NoukaPlacesEne1BBParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
PersonData stage1NoukaPlacesEne1B9Param[] = {
    { 485, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stage1NoukaPlaces[] = {
    { 0, 16, 8, 137, ENE_CONNECT, 2, (u32) &stage1NoukaPlacesEne0FAParam },
    { 0, 15, 0, 121, ENE_DOORS_SET_47, 2, (u32) &stage1NoukaPlacesEne127Param },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 90, 0, 88, ENE_SHOP_KYAKU_OTOKO, 2, (u32) &stage1NoukaPlacesEne1BBParam },
    { 0, 14, 10, -89, ENE_SHOP_KYAKU_OBASAN, 2, (u32) &stage1NoukaPlacesEne1B9Param },
    { 0, 85, 15, -110, ENE_ITEM_OOIRI_MAIN, 2, 400 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};

void stage1NoukaInit(void) {
    gGameWork.geoCacheSize = 40;
    stageBuildFixedCam(0x2A, 0, 0.0f, 0.0f, 0.0f, (u32) stage1NoukaPlaces, 9, 145.0f, 20.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, -5.0f, 0.7853981853f);
}
