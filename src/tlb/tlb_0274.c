#include "tlb.h"

extern ConnectData area1YoroPlacesEne0FAParam0[];
extern ConnectData area1YoroPlacesEne0FAParam1[];
extern PersonData area1YoroPlacesEne104Param[];
extern u16 area1YoroPlacesEne1BBParamPath[];
extern PersonData area1YoroPlacesEne1BBParam[];
ConnectData area1YoroPlacesEne0FAParam0[] = {
    { 15.0f, 25, 0, -1115.0f, 0.0f, -125.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData area1YoroPlacesEne0FAParam1[] = {
    { 15.0f, 25, 0, -1175.0f, 0.0f, -125.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData area1YoroPlacesEne104Param[] = {
    { 736, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
u16 area1YoroPlacesEne1BBParamPath[] = { 0x0000, 0xFFCE, 0x0016, 0x0003, 0x0004, 0x001E, 0x0002, 0x005A, 0x0000, 0x003C, 0x0016, 0x0003, 0x0002, 0x00B4, 0x0004, 0x0096, 0x0002, 0x010E, 0x0001, 0x0000 };
PersonData area1YoroPlacesEne1BBParam[] = {
    { 737, (u32) &area1YoroPlacesEne1BBParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef area1YoroPlaces[] = {
    { 0, 45, 0, 105, ENE_CONNECT, 2, (u32) &area1YoroPlacesEne0FAParam0 },
    { 0, -15, 0, 105, ENE_CONNECT, 2, (u32) &area1YoroPlacesEne0FAParam1 },
    { 0, 0, -1, 30, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_YOROZU_DEBU, 0, 0 },
    { 0, 19, 10, -15, ENE_TABLE_YOROZUYA, 2, (u32) &area1YoroPlacesEne104Param },
    { 0, 44, 13, -13, ENE_SHOP_YOROZU_ITEM1, 2, 0 },
    { 0, 59, 13, -13, ENE_SHOP_YOROZU_ITEM2, 2, 0 },
    { 0, 74, 13, -13, ENE_SHOP_YOROZU_ITEM0, 2, 0 },
    { 0, 0, 1, 0, ENE_SHOP_KYAKU_OTOKO, 2, (u32) &area1YoroPlacesEne1BBParam },
};

void area1YoroInit(void) {
    stageBuildFixedCam(0xF, 0, 0.0f, 0.0f, 0.0f, (u32) area1YoroPlaces, 9, 120.0f, 20.0f, 50.0f);
    camAiSetFar(600);
    camAiSetTurnLimit(40.0f, 36.0f, 0.471238941f);
}
