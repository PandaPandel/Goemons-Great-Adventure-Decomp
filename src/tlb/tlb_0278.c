#include "tlb.h"

extern ConnectData area1ChamisePlacesEne0FAParam[];
extern DoorsData area1ChamisePlacesEne127Param[];
extern u16 area1ChamisePlacesEne19FParamPath[];
extern PersonData area1ChamisePlacesEne19FParam[];
ConnectData area1ChamisePlacesEne0FAParam[] = {
    { 10.0f, 25, 0, 170.0f, 0.0f, -118.0f, 0.0f, 0x0000, 0, 8 },
};
DoorsData area1ChamisePlacesEne127Param[] = {
    { 0.0f, 10 },
};
u16 area1ChamisePlacesEne19FParamPath[] = { 0x0002, 0x010E, 0x0001, 0x0000 };
PersonData area1ChamisePlacesEne19FParam[] = {
    { 755, (u32) &area1ChamisePlacesEne19FParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef area1ChamisePlaces[] = {
    { 0, 32, 0, 100, ENE_CONNECT, 2, (u32) &area1ChamisePlacesEne0FAParam },
    { 0, 31, 0, 79, ENE_DOORS_SET_47, 2, (u32) &area1ChamisePlacesEne127Param },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -50, 18, -12, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 40, 0, 2, ENE_SHOP_CHA_BABA, 2, (u32) &area1ChamisePlacesEne19FParam },
    { 0, 0, 1, 0, ENE_CHAMISE, 0, 0 },
};

void area1ChamiseInit(void) {
    stageBuildFixedCam(0xD, 0, 0.0f, 0.0f, 0.0f, (u32) area1ChamisePlaces, 7, 0.0f, 20.0f, -75.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, -40.0f, 0.628318548f);
    camSunSetTunes4(0x79, 0x76, 0x77, 0x78);
    camSunSetDawnSound(0x42F);
}
