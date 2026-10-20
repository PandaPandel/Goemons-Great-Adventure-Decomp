#include "tlb.h"

extern ConnectData stg2CPlaces303Ene0FAParam[];
extern DoorsData stg2CPlaces303Ene127Param[];
extern u16 stg2CPlaces303Ene140ParamPath[];
extern PersonData stg2CPlaces303Ene140Param[];
ConnectData stg2CPlaces303Ene0FAParam[] = {
    { 10.0f, 50, 0, -112.0f, 80.0f, -135.0f, 1.2566371f, 0x0000, 0, 0 },
};
DoorsData stg2CPlaces303Ene127Param[] = {
    { 0.0f, 19 },
};
u16 stg2CPlaces303Ene140ParamPath[] = { 0x0000, 0x0020, 0x000B, 0x0002, 0x0002, 0x00B4, 0x0000, 0xFFF4, 0x0008, 0x0001, 0x0002, 0x0000, 0x0001, 0x0000 };
PersonData stg2CPlaces303Ene140Param[] = {
    { 534, (u32) &stg2CPlaces303Ene140ParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef stg2CPlaces303[] = {
    { 0, -48, 10, 92, ENE_CONNECT, 2, (u32) &stg2CPlaces303Ene0FAParam },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stg2CPlaces303Ene127Param },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -7, 27, -36, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -12, 12, 8, ENE_MUSUME, 2, (u32) &stg2CPlaces303Ene140Param },
};

void stg2CBuild303(void) {
    stageBuildFixedCam(0x2C, 0, 25.0f, 5.0f, 54.0f, (u32) stg2CPlaces303, 6, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}
