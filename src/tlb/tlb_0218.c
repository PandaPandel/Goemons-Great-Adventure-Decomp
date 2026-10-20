#include "tlb.h"

extern ConnectData castle23PlacesEne0FAParam0[];
extern ConnectData castle23PlacesEne0FAParam1[];
ConnectData castle23PlacesEne0FAParam0[] = {
    { 50.0f, 54, 3, 1140.0f, 200.0f, 0.0f, 0.0f, 0x0000, 0, 4 },
};
ConnectData castle23PlacesEne0FAParam1[] = {
    { 35.0f, 55, 1, 4336.0f, -57.0f, 0.0f, 3.1415927f, 0x0000, 0, 4 },
};
EneDef castle23Places[] = {
    { 1, 284, -60, 0, ENE_CONNECT, 2, (u32) &castle23PlacesEne0FAParam0 },
    { 1, 4307, -50, 0, ENE_CONNECT, 2, (u32) &castle23PlacesEne0FAParam1 },
    { 1, 4250, -293, 0, ENE_ITEM_OOIRI_MAIN, 2, 405 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 1, 760, -189, 0, ENE_ETSK_GOLDNEKO_MAIN_FLAGS_0, 2, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_TAKO_SUMI_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ELEFISH_LIGHTNING, 0, 0 },
    { 1, 580, -160, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 700, -180, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 780, -170, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 900, -160, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 640, -285, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 700, -280, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 820, -240, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 820, -270, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 920, -250, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1090, -160, 0, ENE_ETSK_JELLY_MAIN, 2, 2 },
    { 1, 1120, -260, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 1170, -220, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 1180, -150, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 1250, -180, 0, ENE_ETSK_JELLY_MAIN, 2, 1 },
    { 1, 1310, -240, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 1320, -200, 0, ENE_ETSK_JELLY_MAIN, 2, 2 },
    { 1, 1370, -280, 0, ENE_ETSK_JELLY_MAIN, 2, 2 },
    { 1, 1430, -200, 0, ENE_ETSK_JELLY_MAIN, 2, 2 },
    { 1, 1500, -180, 0, ENE_ETSK_JELLY_MAIN, 2, 1 },
    { 1, 1510, -260, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 1705, -190, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 1780, -165, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1830, -170, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1880, -175, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1930, -180, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1980, -185, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 2030, -190, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1760, -280, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1800, -250, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1825, -280, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1910, -230, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1990, -260, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 2200, -180, 0, ENE_ETSK_JELLY_MAIN, 2, 3 },
    { 1, 2250, -215, 0, ENE_ETSK_JELLY_MAIN, 2, 1 },
    { 1, 2420, -210, 0, ENE_EYEFROG_MAIN, 2, 4 },
    { 1, 2450, -260, 0, ENE_EYEFROG_MAIN, 2, 4 },
    { 1, 2520, -220, 0, ENE_EYEFROG_MAIN, 2, 4 },
    { 1, 2555, -250, 0, ENE_EYEFROG_MAIN, 2, 4 },
    { 1, 2700, -220, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 2750, -240, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 2775, -240, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 2800, -180, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 4 },
    { 1, 2850, -190, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 4 },
    { 1, 2900, -210, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 4 },
    { 1, 2940, -220, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 4 },
    { 1, 2990, -220, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 4 },
    { 1, 3020, -240, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 4 },
    { 1, 3080, -200, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 4 },
    { 1, 3100, -190, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 4 },
    { 1, 3300, -175, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 3250, -200, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 3300, -225, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 3560, -230, 0, ENE_ETSK_JELLY_MAIN, 2, 2 },
    { 1, 3635, -180, 0, ENE_ETSK_JELLY_MAIN, 2, 2 },
    { 1, 3650, -240, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 3750, -190, 0, ENE_ETSK_JELLY_MAIN, 2, 3 },
    { 1, 3780, -230, 0, ENE_ETSK_JELLY_MAIN, 2, 2 },
    { 1, 3860, -270, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 3900, -170, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 3920, -230, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 4000, -210, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 4020, -170, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 4100, -180, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 0, 0, 10, 0, ENE_AQUA, 2, 0 },
    { 0, 0, 10, 0, ENE_WATER_FLAGS_3, 2, 23 },
};

void castle23StageBuild(void) {
    stageBuildDive(0x1E, 1, 350.0f, -290.0f, 0.0f, (u32) castle23Places, 0x4D);
    soundPlayAfterOne(0xA7);
    sndSetReverb(1, 15);
    camAiSetFar(800);
    camSunSetProgram(4);
    gTimeOfDay = 12 << 16;
    camSunSetClockStop(1);
}
