#include "tlb.h"

extern ConnectData stg2CPlaces304Ene0FAParam[];
extern DoorsData stg2CPlaces304Ene127Param[];
extern u16 stg2CPlaces304Ene19FParamPath[];
extern PersonData stg2CPlaces304Ene19FParam[];
ConnectData stg2CPlaces304Ene0FAParam[] = {
    { 10.0f, 50, 0, -89.0f, 80.0f, -194.0f, 1.2566371f, 0x0000, 0, 0 },
};
DoorsData stg2CPlaces304Ene127Param[] = {
    { 0.0f, 19 },
};
u16 stg2CPlaces304Ene19FParamPath[] = { 0x0000, 0x000B, 0x0030, 0x0001, 0x0004, 0x0014, 0x0000, 0xFFDF, 0x0020, 0x0001, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData stg2CPlaces304Ene19FParam[] = {
    { 532, (u32) &stg2CPlaces304Ene19FParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef stg2CPlaces304[] = {
    { 0, -48, 10, 92, ENE_CONNECT, 2, (u32) &stg2CPlaces304Ene0FAParam },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stg2CPlaces304Ene127Param },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -7, 27, -36, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 11, 0, 48, ENE_SHOP_CHA_BABA, 2, (u32) &stg2CPlaces304Ene19FParam },
};

void stg2CBuild304(void) {
    stageBuildFixedCam(0x2C, 0, 0.0f, 10.0f, 0.0f, (u32) stg2CPlaces304, 6, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}
