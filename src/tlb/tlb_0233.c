#include "tlb.h"

#define DPRINTF

extern ConnectData stage57Places2Ene0FAParam0[];
extern ConnectData stage57Places2Ene0FAParam1[];
extern ConnectData stage57JizouPlacesEne0FAParam6[];
extern ConnectData stage57JizouPlacesEne0FAParam7[];
ConnectData stage57Places2Ene0FAParam0[] = {
    { 20.0f, 69, 1, 659.0f, 35.0f, 0.0f, 0.0f, 0x0000, 0, 4 },
};
ConnectData stage57Places2Ene0FAParam1[] = {
    { 20.0f, 69, 1, 4355.0f, 35.0f, 0.0f, 0.0f, 0x0000, 0, 4 },
};
ConnectData stage57JizouPlacesEne0FAParam6[] = {
    { 20.0f, 69, 1, 659.0f, 35.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData stage57JizouPlacesEne0FAParam7[] = {
    { 20.0f, 69, 1, 4355.0f, 35.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef stage57Places[] = {
    { 0, 0, 0, 0, ENE_TAKO_SUMI_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ELEFISH_LIGHTNING, 0, 0 },
    { 0, 0, 0, 0, ENE_POT_FRAGMENT, 0, 0 },
    { 1, 325, 109, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 790, 109, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 1131, 99, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 1298, 99, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 1556, 99, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 1786, 99, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 2015, 111, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 435, 190, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 435, 190, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 620, 150, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 855, 125, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1095, 150, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1210, 135, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1430, 220, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1550, 150, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1985, 170, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1665, 175, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1985, 170, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1740, 100, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1800, 200, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1845, 140, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1985, 170, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 800, 200, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 800, 160, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 800, 120, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1200, 220, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1200, 180, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1200, 140, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1600, 200, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1600, 160, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1600, 120, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 480, 230, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 535, 116, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 648, 238, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 720, 170, 0, ENE_ETSK_JELLY_MAIN, 2, 1 },
    { 1, 761, 100, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 845, 220, 0, ENE_ETSK_JELLY_MAIN, 2, 2 },
    { 1, 967, 139, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 1025, 185, 0, ENE_ETSK_JELLY_MAIN, 2, 3 },
    { 1, 1080, 140, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 1170, 220, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 1288, 163, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 1350, 220, 0, ENE_ETSK_JELLY_MAIN, 2, 1 },
    { 1, 1450, 122, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 1490, 170, 0, ENE_ETSK_JELLY_MAIN, 2, 2 },
    { 1, 1600, 200, 0, ENE_ETSK_JELLY_MAIN, 2, 3 },
    { 1, 1639, 108, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 1715, 220, 0, ENE_ETSK_JELLY_MAIN, 2, 1 },
    { 1, 1790, 160, 0, ENE_ETSK_JELLY_MAIN, 2, 2 },
    { 1, 1885, 200, 0, ENE_ETSK_JELLY_MAIN, 2, 3 },
    { 1, 1925, 140, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 1950, 215, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 384, 168, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 598, 223, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 768, 183, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 947, 205, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1144, 186, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1363, 187, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1554, 193, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1766, 195, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1819, 168, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 2014, 170, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 415, 113, 0, ENE_POT, 2, 0 },
    { 1, 630, 111, 0, ENE_POT, 2, 0 },
    { 1, 1015, 110, 0, ENE_POT, 2, 0 },
    { 0, 0, 329, 0, ENE_WATER_B, 2, 1 },
    { 0, 0, 10, 0, ENE_AQUA, 2, 0 },
};
EneDef stage57Places2[] = {
    { 1, 330, 321, 0, ENE_CONNECT, 2, (u32) &stage57Places2Ene0FAParam0 },
    { 1, 2015, 305, 0, ENE_CONNECT, 2, (u32) &stage57Places2Ene0FAParam1 },
};
EneDef stage57JizouPlaces[] = {
    { 0, 0, 0, 0, ENE_KASA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_JIZOU_FILE_130, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, 0, 0, 0, ENE_KASA_DISP, 0, 0 },
    { 1, 330, 340, 0, ENE_CONNECT, 2, (u32) &stage57JizouPlacesEne0FAParam6 },
    { 1, 2010, 340, 0, ENE_CONNECT, 2, (u32) &stage57JizouPlacesEne0FAParam7 },
};

void stage57Build(void) {
    if (swWorkTest(0xA) && !swWorkTest(0xA3)) {
        stageBuildDive2(0x39, 1, 330.0f, 280.0f, 0.0f, (u32) stage57Places, 0x46, (u32) stage57JizouPlaces, 8);
    } else {
        stageBuildDive2(0x39, 1, 330.0f, 280.0f, 0.0f, (u32) stage57Places, 0x46, (u32) stage57Places2, 2);
    }
    if (swWorkTest(0xA) && !swWorkTest(0xA3)) {
        DPRINTF(">>>>>>>>>>   イベント用セット OK\n");
        eneMakeEx(NULL, ENE_DEMO_JIZOU_FILE_130, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
    }
    camAiSetFar(800);
    camSunSetProgram(4);
    if (!swWorkTest(0xA) || swWorkTest(0xA3)) {
        camSunSetTunes2(0x43, 0x42);
    }
    sndSetReverb(1, 15);
    camSunAddSoundPair(0x131, 0x520);
    camSunAddSoundPair(0x130, 0x520);
}
