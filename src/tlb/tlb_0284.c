#include "tlb.h"

extern ConnectData stage1KoyaPlacesEne0FAParam[];
extern DoorsData stage1KoyaPlacesEne127Param[];
ConnectData stage1KoyaPlacesEne0FAParam[] = {
    { 10.0f, 25, 0, 1091.0f, 0.0f, 150.0f, 0.9424778f, 0x0000, 0, 0 },
};
DoorsData stage1KoyaPlacesEne127Param[] = {
    { 0.0f, 19 },
};
EneDef stage1KoyaPlaces[] = {
    { 0, -48, 10, 92, ENE_CONNECT, 2, (u32) &stage1KoyaPlacesEne0FAParam },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage1KoyaPlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -7, 27, -36, ENE_ROOM_YADOYA_STEAM, 0, 0 },
};

void stage1KoyaInit(void) {
    stageBuildFixedCam(0x2C, 0, 0.0f, 0.0f, 0.0f, (u32) stage1KoyaPlaces, 5, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}
