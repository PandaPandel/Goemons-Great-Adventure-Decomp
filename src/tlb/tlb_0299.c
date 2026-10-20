#include "tlb.h"

extern ConnectData stage2YoroPlacesEne0FAParam0[];
extern ConnectData stage2YoroPlacesEne0FAParam1[];
extern PersonData stage2YoroPlacesEne104Param[];
extern u16 stage2YoroPlacesEne13BParamPath[];
extern PersonData stage2YoroPlacesEne13BParam[];
extern u16 stage2YoroPlacesEne140ParamPath[];
extern PersonData stage2YoroPlacesEne140Param[];
ConnectData stage2YoroPlacesEne0FAParam0[] = {
    { 15.0f, 50, 0, -231.0f, 40.0f, 457.0f, 1.2566371f, 0x0000, 0, 0 },
};
ConnectData stage2YoroPlacesEne0FAParam1[] = {
    { 15.0f, 50, 0, -258.0f, 40.0f, 511.0f, 1.2566371f, 0x0000, 0, 0 },
};
PersonData stage2YoroPlacesEne104Param[] = {
    { 756, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
u16 stage2YoroPlacesEne13BParamPath[] = { 0x0002, 0x00B4, 0x0001, 0x0000 };
PersonData stage2YoroPlacesEne13BParam[] = {
    { 528, (u32) &stage2YoroPlacesEne13BParamPath, { 5, 8 }, 2, 3.1415927f, 1 },
};
u16 stage2YoroPlacesEne140ParamPath[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x0078, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData stage2YoroPlacesEne140Param[] = {
    { 529, (u32) &stage2YoroPlacesEne140ParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef stage2YoroPlaces[] = {
    { 0, 45, 0, 105, ENE_CONNECT, 2, (u32) &stage2YoroPlacesEne0FAParam0 },
    { 0, -15, 0, 105, ENE_CONNECT, 2, (u32) &stage2YoroPlacesEne0FAParam1 },
    { 0, 0, -1, 30, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_YOROZU_DEBU, 0, 0 },
    { 0, 19, 10, -15, ENE_TABLE_YOROZUYA, 2, (u32) &stage2YoroPlacesEne104Param },
    { 0, 44, 13, -13, ENE_SHOP_YOROZU_ITEM2, 2, 0 },
    { 0, 59, 13, -13, ENE_SHOP_YOROZU_ITEM0, 2, 0 },
    { 0, 74, 13, -13, ENE_SHOP_YOROZU_GIN_YOROI, 2, 0 },
    { 0, 71, 2, 4, ENE_ROUNIN, 2, (u32) &stage2YoroPlacesEne13BParam },
    { 0, -58, 2, 31, ENE_MUSUME, 2, (u32) &stage2YoroPlacesEne140Param },
};

void stage2YoroInit(void) {
    stageBuildFixedCam(0xF, 0, 0.0f, 0.0f, 0.0f, (u32) stage2YoroPlaces, 10, 120.0f, 20.0f, 50.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, 36.0f, 0.471238941f);
}
