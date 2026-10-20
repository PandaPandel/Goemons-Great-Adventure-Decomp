#include "tlb.h"

extern ConnectData ryugu214PlacesEne0FAParam0[];
extern ConnectData ryugu214PlacesEne0FAParam1[];
ConnectData ryugu214PlacesEne0FAParam0[] = {
    { 25.0f, 41, 1, 1419.0f, 26.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData ryugu214PlacesEne0FAParam1[] = {
    { 25.0f, 44, 1, 246.0f, 25.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef ryugu214Places[] = {
    { 1, 160, 160, 0, ENE_CONNECT, 2, (u32) &ryugu214PlacesEne0FAParam0 },
    { 1, 680, 160, 0, ENE_CONNECT, 2, (u32) &ryugu214PlacesEne0FAParam1 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 1, 310, 140, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 390, 140, -5, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 460, 140, -5, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 490, 140, -5, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 530, 140, -5, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 0, 0, 10, 0, ENE_AQUA, 2, 0 },
};

void ryugu214StageBuild(void) {
    stageBuildBasic(0x15, 1, 215.0f, 135.0f, 0.0f, (u32) ryugu214Places, 0xD);
    camSunSetTunes5(0x518, 0x3E, 0x3F, 0x40, 0x41);
    sndSetReverb(1, 15);
    camSunAddSoundPair(0x28D, 0x520);
    camSunAddSoundPair(0x12F, 0x520);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0xC3);
    camAiAddLimit(1, 1, 0x27B);
    camSunSetProgram(4);
}
