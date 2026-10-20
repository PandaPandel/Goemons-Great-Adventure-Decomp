#include "tlb.h"

extern ConnectData ryugu212PlacesEne0FAParam0[];
extern ConnectData ryugu212PlacesEne0FAParam3[];
extern ConnectData ryugu212PlacesEne0FAParam1[];
extern ConnectData ryugu212PlacesEne0FAParam2[];
ConnectData ryugu212PlacesEne0FAParam0[] = {
    { 50.0f, 40, 1, 1490.0f, 140.0f, 0.0f, 1313.0f, 0x0000, 0, 0 },
};
ConnectData ryugu212PlacesEne0FAParam3[] = {
    { 50.0f, 43, 1, 196.0f, 140.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData ryugu212PlacesEne0FAParam1[] = {
    { 100.0f, 42, 1, 100.0f, 80.0f, 0.0f, 0.0f, 0x0000, 0, 2 },
};
ConnectData ryugu212PlacesEne0FAParam2[] = {
    { 100.0f, 42, 1, 390.0f, 80.0f, 0.0f, 0.0f, 0x0000, 0, 2 },
};
EneDef ryugu212Places[] = {
    { 1, 209, 4, 0, ENE_CONNECT, 2, (u32) &ryugu212PlacesEne0FAParam0 },
    { 1, 740, -131, 0, ENE_CONNECT, 2, (u32) &ryugu212PlacesEne0FAParam1 },
    { 1, 1082, -131, 0, ENE_CONNECT, 2, (u32) &ryugu212PlacesEne0FAParam2 },
    { 1, 1470, 26, 0, ENE_CONNECT, 2, (u32) &ryugu212PlacesEne0FAParam3 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_BOATSKEL_HEAD_MAIN, 0, 0 },
    { 1, 400, 26, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 430, 26, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 500, 26, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 620, 4, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 2 },
    { 1, 650, 26, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 890, 4, 20, ENE_BOATSKEL_HEAD_MAIN, 2, 0 },
    { 1, 920, 4, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1115, -15, -55, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 1195, 4, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1205, 4, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1225, 4, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1245, 4, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 0, 0, -8, 0, ENE_OBJ22_WATER_FLAGS_3, 2, 3 },
};

void ryugu212StageBuild(void) {
    stageBuildBasic(0x13, 1, 210.0f, 5.0f, 0.0f, (u32) ryugu212Places, 0x15);
    camSunSetTunes5(0x518, 0x3E, 0x3F, 0x40, 0x41);
    sndSetReverb(1, 15);
    camSunAddSoundPair(0x28D, 0x520);
    camSunAddSoundPair(0x12F, 0x520);
    camAiSetFar(800);
    camSunSetProgram(8);
}
