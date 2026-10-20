#include "tlb.h"

extern ConnectData stage2ChamisePlacesEne0FAParam[];
extern DoorsData stage2ChamisePlacesEne127Param[];
extern u32 stage2ChamisePlacesEne19FParamPath;
extern PersonData stage2ChamisePlacesEne19FParam[];
ConnectData stage2ChamisePlacesEne0FAParam[] = {
    { 10.0f, 50, 0, -476.0f, 22.0f, 726.0f, 0.31415927f, 0x0000, 0, 8 },
};
DoorsData stage2ChamisePlacesEne127Param[] = {
    { 0.0f, 10 },
};
u32 stage2ChamisePlacesEne19FParamPath = 0x00010000;
PersonData stage2ChamisePlacesEne19FParam[] = {
    { 765, (u32) &stage2ChamisePlacesEne19FParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stage2ChamisePlaces[] = {
    { 0, 32, 0, 100, ENE_CONNECT, 2, (u32) &stage2ChamisePlacesEne0FAParam },
    { 0, 31, 0, 79, ENE_DOORS_SET_47, 2, (u32) &stage2ChamisePlacesEne127Param },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -50, 18, -12, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 40, 0, 2, ENE_SHOP_CHA_BABA, 2, (u32) &stage2ChamisePlacesEne19FParam },
    { 0, 0, 1, 0, ENE_CHAMISE, 0, 0 },
};

void stage2ChamiseInit(void) {
    stageBuildFixedCam(0xD, 0, 0.0f, 0.0f, 0.0f, (u32) stage2ChamisePlaces, 7, 0.0f, 20.0f, -75.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, -40.0f, 0.6283185482f);
    camSunSetTunes4(0x96, 0x93, 0x94, 0x95);
    camSunSetDawnSound(0x42F);
}
