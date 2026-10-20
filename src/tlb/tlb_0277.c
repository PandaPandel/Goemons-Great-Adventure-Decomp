#include "tlb.h"

extern ConnectData area1YadoPlacesEne0FAParam[];
extern PersonData area1YadoPlacesEne19DParam[];
extern DoorsData area1YadoPlacesEne127Param[];
extern u16 area1YadoPlacesEne1BAParamPath[];
extern PersonData area1YadoPlacesEne1BAParam[];
ConnectData area1YadoPlacesEne0FAParam[] = {
    { 10.0f, 25, 0, -354.0f, 0.0f, -150.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData area1YadoPlacesEne19DParam[] = {
    { 750, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
DoorsData area1YadoPlacesEne127Param[] = {
    { 0.0f, 13 },
};
u16 area1YadoPlacesEne1BAParamPath[] = { 0x0000, 0x000D, 0x0020, 0x0001, 0x0004, 0x003C, 0x0002, 0x00B4, 0x0000, 0x000D, 0xFFF6, 0x0001, 0x0002, 0x005A, 0x0004, 0x003C, 0x0002, 0x0000, 0x0001, 0x0000 };
PersonData area1YadoPlacesEne1BAParam[] = {
    { 754, (u32) &area1YadoPlacesEne1BAParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef area1YadoPlaces[] = {
    { 0, 60, 0, 70, ENE_CONNECT, 2, (u32) &area1YadoPlacesEne0FAParam },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -29, 27, 6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 60, 0, 48, ENE_DOORS_SET_47, 2, (u32) &area1YadoPlacesEne127Param },
    { 0, 0, 0, 0, ENE_SHOP_YADOYA, 0, 0 },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 57, 4, -32, ENE_TABLE_YADOYA, 2, (u32) &area1YadoPlacesEne19DParam },
    { 0, 0, 1, 0, ENE_SHOP_KYAKU_BOUZU, 2, (u32) &area1YadoPlacesEne1BAParam },
};

void area1YadoInit(void) {
    stageBuildFixedCam(0x10, 0, 0.0f, 0.0f, 0.0f, (u32) area1YadoPlaces, 8, 120.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, 5.0f, 0.50265485f);
}
