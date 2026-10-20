#include "tlb.h"

extern ConnectData stg11Places310Ene0FAParam[];
extern DoorsData stg11Places310Ene127Param[];
extern PersonData stg11Places310Ene179Param[];
extern u16 stg11Places310Ene17FParamPath[];
extern PersonData stg11Places310Ene17FParam[];
ConnectData stg11Places310Ene0FAParam[] = {
    { 15.0f, 71, 0, 95.0f, 140.0f, -20.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stg11Places310Ene127Param[] = {
    { 0.0f, 15 },
};
PersonData stg11Places310Ene179Param[] = {
    { 0, 0x00000000, { 5, 8 }, 2, 0.87266463f, 0 },
};
u16 stg11Places310Ene17FParamPath[] = { 0x0002, 0x001E, 0x0004, 0x0014, 0x0002, 0x0000, 0x0007, 0x0002, 0x0008, 0x0001, 0x0007, 0x0002, 0x0008, 0x0000, 0x0004, 0x0014, 0x0002, 0x014A, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData stg11Places310Ene17FParam[] = {
    { 555, (u32) &stg11Places310Ene17FParamPath, { 5, 8 }, 1, 0.0f, 0 },
};
EneDef stg11Places310[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &stg11Places310Ene0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stg11Places310Ene127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -29, 10, 2, ENE_HANAOYA, 2, (u32) &stg11Places310Ene179Param },
    { 0, 15, 10, -29, ENE_DESHI_PERSON, 2, (u32) &stg11Places310Ene17FParam },
    { 0, 26, 25, -41, ENE_BLACK, 2, 2 },
    { 0, 0, 0, 0, ENE_DEMO_KAKURENBO, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
};

void stg11Build310(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) stg11Places310, 0xA, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
    eneMakeEx(NULL, ENE_DEMO_KAKURENBO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
}
