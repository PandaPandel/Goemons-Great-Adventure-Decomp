#include "tlb.h"

extern ConnectData stg11Places311Ene0FAParam[];
extern DoorsData stg11Places311Ene127Param[];
extern u16 stg11Places311Ene13EParamPath[];
extern PersonData stg11Places311Ene13EParam[];
ConnectData stg11Places311Ene0FAParam[] = {
    { 15.0f, 71, 0, -28.0f, 140.0f, -36.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stg11Places311Ene127Param[] = {
    { 0.0f, 15 },
};
u16 stg11Places311Ene13EParamPath[] = { 0x0002, 0x001E, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0002, 0x0000, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData stg11Places311Ene13EParam[] = {
    { 585, (u32) &stg11Places311Ene13EParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stg11Places311[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &stg11Places311Ene0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stg11Places311Ene127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -17, 10, -6, ENE_KODOMO, 2, (u32) &stg11Places311Ene13EParam },
};

void stg11Build311(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) stg11Places311, 6, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
}
