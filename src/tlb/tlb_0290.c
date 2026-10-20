#include "tlb.h"

extern ConnectData stageTutiZiziPlacesEne0FAParam[];
extern DoorsData stageTutiZiziPlacesEne127Param[];
extern PersonData stageTutiZiziPlacesEne15DParam[];
ConnectData stageTutiZiziPlacesEne0FAParam[] = {
    { 10.0f, 26, 0, -84.0f, 0.0f, 16.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stageTutiZiziPlacesEne127Param[] = {
    { 0.0f, 19 },
};
PersonData stageTutiZiziPlacesEne15DParam[] = {
    { 445, 0x00000000, { 4, 8 }, 0, 0.0f, 0 },
};
EneDef stageTutiZiziPlaces[] = {
    { 0, -48, 10, 92, ENE_CONNECT, 2, (u32) &stageTutiZiziPlacesEne0FAParam },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stageTutiZiziPlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -7, 27, -36, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -22, 20, -8, ENE_TSUCHI_JISAN, 2, (u32) &stageTutiZiziPlacesEne15DParam },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};

void stageTutiZiziInit(void) {
    stageBuildFixedCam(0x2C, 0, 0.0f, 10.0f, 0.0f, (u32) stageTutiZiziPlaces, 9, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}
