#include "tlb.h"

extern ConnectData mafuu323PlacesEne0FAParam0[];
extern ConnectData mafuu323PlacesEne0FAParam1[];
ConnectData mafuu323PlacesEne0FAParam0[] = {
    { 20.0f, 61, 1, 1850.0f, 20.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData mafuu323PlacesEne0FAParam1[] = {
    { 20.0f, 61, 5, 80.0f, 300.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef mafuu323Places[] = {
    { 1, 40, 110, 0, ENE_CONNECT, 2, (u32) &mafuu323PlacesEne0FAParam0 },
    { 1, 2700, 70, 0, ENE_CONNECT, 2, (u32) &mafuu323PlacesEne0FAParam1 },
    { 0, -244, -5, 380, ENE_MAGMA_FLAGS_3, 2, -5111794 },
    { 0, 256, 0, -20, ENE_KPAKO, 2, 838 },
    { 0, -176, 0, 720, ENE_KPAKO, 2, 476 },
    { 0, -242, 0, 698, ENE_KPAKO, 2, 476 },
    { 1, 2104, 44, 0, ENE_KPAOYA, 2, 220 },
};
EneDef mafuu323PlacesNormal[] = {
    { 1, 2089, 46, 0, ENE_ITEM_OOIRI_MAIN, 2, 406 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 0, 0 },
    { 0, 0, 0, 0, ENE_A3KP, 2, 0 },
    { 1, 125, 50, 0, ENE_A3KP_GEN, 2, 524288 },
    { 1, 216, 60, 0, ENE_A3KP, 2, 262145 },
    { 1, 258, 50, 0, ENE_A3KP, 2, 131073 },
    { 1, 305, -6, 0, ENE_A3KP_GEN, 2, 524288 },
    { 1, 332, 28, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 4 },
    { 1, 425, -9, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 10 },
    { 1, 535, 25, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 4 },
    { 1, 592, 25, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 4 },
    { 1, 695, 35, 0, ENE_A3KP, 2, 131073 },
    { 1, 755, 62, 0, ENE_A3KP, 2, 196609 },
    { 1, 835, 80, 0, ENE_A3KP, 2, 262145 },
    { 1, 895, 33, 0, ENE_A3KP_GEN, 2, 524288 },
    { 1, 1015, 55, 0, ENE_A3KP, 2, 262145 },
    { 1, 1125, -6, 0, ENE_A3KP_GEN, 2, 524288 },
    { 1, 1245, 70, 0, ENE_A3KP, 2, 262145 },
    { 1, 1300, 65, 0, ENE_A3KP_BEARD, 2, 262147 },
    { 1, 1450, 30, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 4 },
    { 1, 1533, -9, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 10 },
    { 1, 1603, -9, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 10 },
    { 1, 1700, 15, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 4 },
    { 1, 1755, 40, 0, ENE_A3KP, 2, 262145 },
    { 1, 1785, 43, 0, ENE_A3KP_BEARD, 2, 262147 },
    { 1, 1850, 55, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 4 },
    { 1, 1895, 55, 0, ENE_A3KP, 2, 262145 },
    { 1, 2505, 55, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 5 },
    { 1, 2565, 55, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 5 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 1, 2257, 37, 0, ENE_A3KP_BEARD, 2, 262147 },
    { 1, 2390, 37, 0, ENE_A3KP_BEARD, 2, 262147 },
    { 1, 2257, 37, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 2610, -6, 0, ENE_A3KP_GEN, 2, 524288 },
};
EneDef mafuu323PlacesEvent[] = {
    { 0, 0, 0, 0, ENE_DEMO_KINOKO, 0, 0 },
    { 1, 427, 77, 0, ENE_KINOKO, 2, 280 },
    { 1, 2463, 41, 0, ENE_KINOKO, 2, 25 },
    { 1, 2525, 46, 0, ENE_KINOKO, 2, 26 },
    { 1, 2593, 51, 0, ENE_KINOKO, 2, 27 },
    { 1, 1466, 62, 0, ENE_KINOKO, 2, 284 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, 0, 0, 0, ENE_KINOKO_DISP, 0, 0 },
};

void mafuu323StageBuild(void) {
    if (swWorkTest(0xD) && !swWorkTest(0xA4)) {
        stageBuildBasic2(0x33, 1, 65.0f, 95.0f, 0.0f, (u32) mafuu323Places, 7, (u32) mafuu323PlacesEvent, 0xA);
        PRINTF("イベント用セット OK\n");
    } else {
        stageBuildBasic2Atk150(0x33, 1, 65.0f, 95.0f, 0.0f, (u32) mafuu323Places, 7, (u32) mafuu323PlacesNormal, 0x25);
    }
    camAiSetFar(800);
    camSunSetProgram(8);
    if (!swWorkTest(0xD) || swWorkTest(0xA4)) {
        camSunSetTunes5(0x518, 0x50, 0x51, 0x52, 0x53);
    }
    sndSetReverb(1, 15);
    camSunAddSoundPair(0x131, 0x520);
    camSunAddSoundPair(0x130, 0x520);
    eneMakeEx(NULL, ENE_DEMO_KINOKO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
}
