#include "tlb.h"

extern ConnectData stg2EPlaces306Ene0FAParam0[];
extern ConnectData stg2EPlaces306Ene0FAParam1[];
extern ConnectData stg2EPlaces306Ene0FAParam2[];
extern PersonData stg2EPlaces306Ene128Param[];
ConnectData stg2EPlaces306Ene0FAParam0[] = {
    { 10.0f, 50, 0, 409.0f, 160.0f, -544.0f, 0.0f, 0x0007, 0, 0 },
};
ConnectData stg2EPlaces306Ene0FAParam1[] = {
    { 10.0f, 50, 0, 463.0f, 160.0f, -544.0f, 0.0f, 0x0007, 0, 0 },
};
ConnectData stg2EPlaces306Ene0FAParam2[] = {
    { 10.0f, 50, 0, 515.0f, 160.0f, -544.0f, 0.0f, 0x0007, 0, 0 },
};
PersonData stg2EPlaces306Ene128Param[] = {
    { 40, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stg2EPlaces306[] = {
    { 0, -40, 10, 135, ENE_CONNECT, 2, (u32) &stg2EPlaces306Ene0FAParam0 },
    { 0, 0, 10, 135, ENE_CONNECT, 2, (u32) &stg2EPlaces306Ene0FAParam1 },
    { 0, 40, 10, 135, ENE_CONNECT, 2, (u32) &stg2EPlaces306Ene0FAParam2 },
    { 0, 0, -1, -80, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_PURA, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_DANCER, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_DANCER, 0, 0 },
    { 0, 0, 0, 0, ENE_URANAI_DECO, 0, 0 },
    { 0, 0, 0, 0, ENE_URANAI_CAMERA, 0, 0 },
    { 0, 40, 5, 0, ENE_URANAI_TABLE, 2, (u32) &stg2EPlaces306Ene128Param },
};

void stg2EBuild306(void) {
    stageBuildFixedCam(0x2E, 0, 0.0f, 0.0f, 77.0f, (u32) stg2EPlaces306, 0xA, -180.0f, 20.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(10.0f, 0.0f, 0.471238941f);
}
