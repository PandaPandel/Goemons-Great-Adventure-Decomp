#include "tlb.h"

extern ConnectData castle22PlacesEne0FAParam0[];
extern ConnectData castle22PlacesEne0FAParam1[];
ConnectData castle22PlacesEne0FAParam0[] = {
    { 70.0f, 51, 5, 180.0f, 170.0f, 0.0f, 0.0f, 0x0000, 0, 4 },
};
ConnectData castle22PlacesEne0FAParam1[] = {
    { 60.0f, 54, 1, 1620.0f, 170.0f, 0.0f, 0.0f, 0x0000, 0, 4 },
};
EneDef castle22Places[] = {
    { 1, 2120, -50, 0, ENE_CONNECT, 2, (u32) &castle22PlacesEne0FAParam0 },
    { 1, 170, -60, 0, ENE_CONNECT, 2, (u32) &castle22PlacesEne0FAParam1 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_POT_FRAGMENT, 0, 0 },
    { 0, 0, 0, 0, ENE_TAKO_SUMI_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ELEFISH_LIGHTNING, 0, 0 },
    { 1, 2050, -270, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1970, -190, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1850, -230, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1873, -320, 0, ENE_POT, 2, 0 },
    { 1, 1837, -290, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 1683, -138, 0, ENE_ETSK_UTSUBO_MAIN, 2, 0 },
    { 1, 1567, -168, 0, ENE_ETSK_UTSUBO_MAIN, 2, 0 },
    { 1, 1530, -200, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1400, -240, 0, ENE_ETSK_UTSUBO_MAIN, 2, 0 },
    { 1, 1310, -230, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 1250, -190, 0, ENE_ETSK_JELLY_MAIN, 2, 1 },
    { 1, 1240, -260, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1170, -230, 0, ENE_ETSK_JELLY_MAIN, 2, 2 },
    { 1, 1100, -150, 0, ENE_ETSK_JELLY_MAIN, 2, 3 },
    { 1, 1160, -315, 0, ENE_POT, 2, 0 },
    { 1, 1120, -190, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1040, -165, 0, ENE_ETSK_UTSUBO_MAIN, 2, 0 },
    { 1, 905, -190, 0, ENE_ETSK_UTSUBO_MAIN, 2, 0 },
    { 1, 878, -265, 0, ENE_ETSK_UTSUBO_MAIN, 2, 0 },
    { 1, 860, -225, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 800, -288, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 487, -308, 0, ENE_ETSK_YADO_MAIN, 2, 1 },
    { 1, 645, -230, 0, ENE_ETSK_JELLY_MAIN, 2, 3 },
    { 1, 600, -255, 0, ENE_ETSK_JELLY_MAIN, 2, 3 },
    { 1, 520, -180, 0, ENE_ETSK_JELLY_MAIN, 2, 3 },
    { 1, 395, -240, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 215, -250, 0, ENE_ETSK_JELLY_MAIN, 2, 3 },
    { 0, 0, 10, 0, ENE_AQUA, 2, 0 },
    { 0, 0, 10, 0, ENE_WATER_FLAGS_3, 2, 22 },
};

void castle22StageBuild(void) {
    stageBuildDive(0x1D, 1, 2100.0f, -180.0f, 0.0f, (u32) castle22Places, 0x25);
    soundPlayAfterOne(0xA7);
    sndSetReverb(1, 15);
    camAiSetFar(800);
    camSunSetProgram(4);
    gTimeOfDay = 12 << 16;
    camSunSetClockStop(1);
}
