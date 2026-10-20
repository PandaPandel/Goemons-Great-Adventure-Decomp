#include "tlb.h"

extern ConnectData stage2Minka2PlacesEne0FAParam[];
extern DoorsData stage2Minka2PlacesEne127Param[];
extern PersonData stage2Minka2PlacesEne16EParam[];
ConnectData stage2Minka2PlacesEne0FAParam[] = {
    { 10.0f, 50, 0, -152.0f, 70.0f, 44.0f, 1.5707964f, 0x0000, 0, 0 },
};
DoorsData stage2Minka2PlacesEne127Param[] = {
    { 0.0f, 19 },
};
PersonData stage2Minka2PlacesEne16EParam[] = {
    { 0, 0x00000000, { 8, 8 }, 2, 0.0f, 0 },
};
EneDef stage2Minka2Places[] = {
    { 0, -48, 10, 92, ENE_CONNECT, 2, (u32) &stage2Minka2PlacesEne0FAParam },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage2Minka2PlacesEne127Param },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -7, 27, -36, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -39, 10, -39, ENE_OBISU_MARU, 2, (u32) &stage2Minka2PlacesEne16EParam },
    { 0, 0, 0, 0, ENE_OBISU_EFFECT, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};

void stage2Minka2Init(void) {
    stageBuildFixedCam(0x2C, 0, 0.0f, 10.0f, 0.0f, (u32) stage2Minka2Places, 10, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}
