#include "tlb.h"

extern ConnectData stg11Places312Ene0FAParam[];
extern DoorsData stg11Places312Ene127Param[];
extern u16 stg11Places312Ene187ParamPath[];
extern PersonData stg11Places312Ene187Param[];
ConnectData stg11Places312Ene0FAParam[] = {
    { 15.0f, 71, 0, -123.0f, 140.0f, -79.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stg11Places312Ene127Param[] = {
    { 0.0f, 15 },
};
u16 stg11Places312Ene187ParamPath[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData stg11Places312Ene187Param[] = {
    { 584, (u32) &stg11Places312Ene187ParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef stg11Places312[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &stg11Places312Ene0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stg11Places312Ene127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -20, 12, 0, ENE_KODOMO_PINK, 2, (u32) &stg11Places312Ene187Param },
};

void stg11Build312(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) stg11Places312, 6, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
}
