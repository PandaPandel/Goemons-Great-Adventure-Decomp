#include "tlb.h"

extern ConnectData stage1BeatroomPlacesEne0FAParam[];
extern DoorsData stage1BeatroomPlacesEne127Param[];
extern PersonData stage1BeatroomPlacesEne161Param[];
ConnectData stage1BeatroomPlacesEne0FAParam[] = {
    { 15.0f, 25, 0, 548.0f, 0.0f, -64.0f, -0.31415927f, 0x0000, 0, 0 },
};
DoorsData stage1BeatroomPlacesEne127Param[] = {
    { 0.0f, 15 },
};
PersonData stage1BeatroomPlacesEne161Param[] = {
    { 442, 0x00000000, { 4, 8 }, 0, 0.0f, 0 },
};
EneDef stage1BeatroomPlaces[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &stage1BeatroomPlacesEne0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage1BeatroomPlacesEne127Param },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -12, 15, -1, ENE_BEAT_MANIA, 2, (u32) &stage1BeatroomPlacesEne161Param },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
};

void stage1BeatroomInit(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) stage1BeatroomPlaces, 9, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
}
