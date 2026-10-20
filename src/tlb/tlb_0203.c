#include "tlb.h"

extern ConnectData stg0203PlacesEne0FAParam12[];
extern ConnectData stg0203PlacesEne0FAParam13[];
ConnectData stg0203PlacesEne0FAParam12[] = {
    { 40.0f, 39, 1, 287.0f, 200.0f, 0.0f, 0.0f, 0x000A, 0, 0 },
};
ConnectData stg0203PlacesEne0FAParam13[] = {
    { 50.0f, 32, 5, 720.0f, 1055.0f, 0.0f, 0.0f, 0x0000, 4, 0 },
};
EneDef stg0203Places[] = {
    { 1, 1363, 151, 0, ENE_ITEM_OOIRI_MAIN, 2, 402 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 2, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 2, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 2, 0 },
    { 0, 0, 0, 0, ENE_POT_FRAGMENT, 0, 0 },
    { 1, 185, 150, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 460, 170, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 670, 160, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 2, 1 },
    { 1, 860, 150, 0, ENE_POT, 2, 0 },
    { 1, 1900, 190, 0, ENE_CONNECT, 2, (u32) &stg0203PlacesEne0FAParam12 },
    { 1, 242, 60, 0, ENE_CONNECT, 2, (u32) &stg0203PlacesEne0FAParam13 },
    { 0, -1775, -106, -150, ENE_GIM_IHEAD, 2, 256 },
    { 0, 164, 90, -150, ENE_GIM_ELEVATOR, 2, 983095 },
    { 0, -600, 160, 0, ENE_GIM_FIREWALL5, 2, 96 },
    { 0, -622, 160, -100, ENE_GIM_FIREWALL4, 2, 0 },
    { 0, -1346, 160, -50, ENE_GIM_FIREWALL4, 2, 0 },
    { 0, -1416, 150, -130, ENE_GIM_FIREWALL5, 2, 0 },
};

void stg0203Build(void) {
    stageBuildBasic(0xB, 1, 300.0f, 170.0f, 0.0f, (u32) stg0203Places, 0x14);
    if (gGameWork.tunePart < 4) {
        soundPlayAfterOne(0xA4);
        gGameWork.tunePart = 3;
    }
    sndPlayVol(0x29D, 0x87);
    camAiSetFar(600);
    gTimeOfDay = 18 << 16;
    camSunRefreshHold(1);
    gGameState.fade = 1;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
    gGameState.fadeStart = 0x30;
    gGameState.fadeTo[0] = 0xA0;
    gGameState.fadeTo[1] = 0x10;
    gGameState.fadeTo[2] = 0x10;
    gGameState.fadeEnd = 0x80;
}
