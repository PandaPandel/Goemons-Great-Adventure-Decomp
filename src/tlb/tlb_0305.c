#include "tlb.h"

extern ConnectData stg2CPlaces305Ene0FAParam[];
extern DoorsData stg2CPlaces305Ene127Param[];
extern u16 stg2CPlaces305Ene1BAParamPath[];
extern PersonData stg2CPlaces305Ene1BAParam[];
ConnectData stg2CPlaces305Ene0FAParam[] = {
    { 10.0f, 50, 0, -61.0f, 80.0f, -250.0f, 1.2566371f, 0x0000, 0, 0 },
};
DoorsData stg2CPlaces305Ene127Param[] = {
    { 0.0f, 19 },
};
u16 stg2CPlaces305Ene1BAParamPath[] = { 0x0002, 0x005A, 0x0004, 0x003C, 0x0002, 0x0000, 0x0004, 0x003C, 0x0001, 0x0000 };
PersonData stg2CPlaces305Ene1BAParam[] = {
    { 533, (u32) &stg2CPlaces305Ene1BAParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef stg2CPlaces305[] = {
    { 0, -48, 10, 92, ENE_CONNECT, 2, (u32) &stg2CPlaces305Ene0FAParam },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stg2CPlaces305Ene127Param },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -7, 27, -36, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -27, 10, -36, ENE_SHOP_KYAKU_BOUZU, 2, (u32) &stg2CPlaces305Ene1BAParam },
};

void stg2CBuild305(void) {
    stageBuildFixedCam(0x2C, 0, 0.0f, 10.0f, 0.0f, (u32) stg2CPlaces305, 6, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}
