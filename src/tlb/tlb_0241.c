#include "tlb.h"

extern ConnectData stage58PlacesEne0FAParam12[];
extern ConnectData stage58PlacesEne0FAParam14[];
ConnectData stage58PlacesEne0FAParam12[] = {
    { 70.0f, 81, 2, 246.0f, -5.0f, 0.0f, 0.0f, 0x0000, 0, 4 },
};
ConnectData stage58PlacesEne0FAParam14[] = {
    { 40.0f, 83, 1, 450.0f, -130.0f, 0.0f, 0.0f, 0x0000, 0, 4 },
};
EneDef stage58Places[] = {
    { 0, 600, 350, 500, ENE_WATER_RIDE_FLAGS_3, 2, 6 },
    { 0, 0, 10, 0, ENE_AQUA, 2, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_POT_FRAGMENT, 0, 0 },
    { 1, 670, 230, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1000, 250, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1390, 100, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1750, 120, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 2060, 110, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 2270, 90, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 255, 377, 0, ENE_CONNECT, 2, (u32) &stage58PlacesEne0FAParam12 },
    { 1, 385, 377, 0, ENE_CONNECT, 2, (u32) &stage58PlacesEne0FAParam12 },
    { 1, 2469, 377, 0, ENE_CONNECT, 2, (u32) &stage58PlacesEne0FAParam14 },
};

void stage58Build(void) {
    stageBuildDive(0x3A, 1, 350.0f, 300.0f, 0.0f, (u32) stage58Places, 0xF);
    camAiSetFar(800);
    camSunSetProgram(4);
    camSunSetTunes2(0x4B, 0x4A);
    sndSetReverb(1, 15);
    camSunAddSoundPair(0x28C, 0x520);
    camSunAddSoundPair(0x12F, 0x520);
}
