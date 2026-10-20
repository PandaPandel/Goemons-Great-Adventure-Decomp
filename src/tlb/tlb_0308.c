#include "tlb.h"

extern ConnectData stg2CPlaces308Ene0FAParam[];
extern DoorsData stg2CPlaces308Ene127Param[];
extern PersonData stg2CPlaces308Ene16AParam[];
ConnectData stg2CPlaces308Ene0FAParam[] = {
    { 10.0f, 71, 0, 221.0f, -59.0f, 248.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stg2CPlaces308Ene127Param[] = {
    { 0.0f, 19 },
};
PersonData stg2CPlaces308Ene16AParam[] = {
    { 0, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stg2CPlaces308[] = {
    { 0, -48, 10, 92, ENE_CONNECT, 2, (u32) &stg2CPlaces308Ene0FAParam },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stg2CPlaces308Ene127Param },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -7, 27, -36, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, -21, 10, 0, ENE_KINOKO_MANIA, 2, (u32) &stg2CPlaces308Ene16AParam },
    { 0, 0, 0, 0, ENE_EPRG_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};

void stg2CBuild308(void) {
    stageBuildFixedCam(0x2C, 0, 0.0f, 0.0f, 0.0f, (u32) stg2CPlaces308, 0xB, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}
