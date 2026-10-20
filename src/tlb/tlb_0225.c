#include "tlb.h"

extern ConnectData mafuu322PlacesEne0FAParam0[];
extern ConnectData mafuu322PlacesEne0FAParam1[];
ConnectData mafuu322PlacesEne0FAParam0[] = {
    { 20.0f, 61, 3, 320.0f, 100.0f, 0.0f, 0.0f, 0x0521, 0, 0 },
};
ConnectData mafuu322PlacesEne0FAParam1[] = {
    { 20.0f, 61, 4, 50.0f, 290.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef mafuu322Places[] = {
    { 1, 48, 80, 0, ENE_CONNECT, 2, (u32) &mafuu322PlacesEne0FAParam0 },
    { 1, 1810, 330, 0, ENE_CONNECT, 2, (u32) &mafuu322PlacesEne0FAParam1 },
    { 1, 1627, 353, 0, ENE_BPOLE, 2, 0 },
    { 1, 1687, 353, 0, ENE_BPOLE, 2, 0 },
    { 0, 0, 46, 0, ENE_MAGMA_FLAGS_3, 2, 14 },
};
EneDef mafuu322PlacesNormal[] = {
    { 1, 215, 106, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 260, 106, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 230, 160, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 1, 1515, 317, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 1570, 317, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 1550, 370, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 1, 350, 121, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 375, 126, 0, ENE_ETSK_BETO_MAIN, 2, 2 },
    { 1, 400, 131, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 450, 138, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 500, 146, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 550, 155, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 575, 158, 0, ENE_ETSK_BETO_MAIN, 2, 2 },
    { 1, 600, 163, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 650, 171, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 700, 180, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 750, 188, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 800, 193, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 825, 201, 0, ENE_ETSK_BETO_MAIN, 2, 2 },
    { 1, 850, 206, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 900, 213, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 950, 222, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 975, 225, 0, ENE_ETSK_BETO_MAIN, 2, 2 },
    { 1, 1000, 229, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1050, 237, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1100, 246, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1150, 253, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1200, 261, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1225, 265, 0, ENE_ETSK_BETO_MAIN, 2, 2 },
    { 1, 1250, 270, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1300, 277, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 1350, 285, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1375, 289, 0, ENE_ETSK_BETO_MAIN, 2, 2 },
    { 1, 1400, 292, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
};
EneDef mafuu322PlacesEvent[] = {
    { 0, 0, 0, 0, ENE_DEMO_KINOKO, 0, 0 },
    { 1, 365, 123, 0, ENE_KINOKO, 2, 13 },
    { 1, 456, 137, 0, ENE_KINOKO, 2, 14 },
    { 1, 565, 155, 0, ENE_KINOKO, 2, 15 },
    { 1, 741, 185, 0, ENE_KINOKO, 2, 16 },
    { 1, 855, 204, 0, ENE_KINOKO, 2, 17 },
    { 1, 1040, 255, 0, ENE_KINOKO, 2, 274 },
    { 1, 1197, 298, 0, ENE_KINOKO, 2, 275 },
    { 1, 1380, 327, 0, ENE_KINOKO, 2, 276 },
    { 1, 1459, 334, 0, ENE_KINOKO, 2, 277 },
    { 1, 1616, 411, 0, ENE_KINOKO, 2, 22 },
    { 1, 1689, 409, 0, ENE_KINOKO, 2, 23 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, 0, 0, 0, ENE_KINOKO_DISP, 0, 0 },
};

void mafuu322StageBuild(void) {
    if (swWorkTest(0xD) && !swWorkTest(0xA4)) {
        stageBuildBasic2(0x32, 1, 100.0f, 85.0f, 0.0f, (u32) mafuu322Places, 5, (u32) mafuu322PlacesEvent, 0x10);
        PRINTF("イベント用セット OK\n");
    } else {
        stageBuildBasic2Atk150(0x32, 1, 100.0f, 85.0f, 0.0f, (u32) mafuu322Places, 5, (u32) mafuu322PlacesNormal, 0x23);
    }
    camAiSetFar(800);
    camSunSetProgram(8);
    camAiAddLimit(1, -1, 0x69);
    camAiAddLimit(1, 1, 0x6DB);
    if (!swWorkTest(0xD) || swWorkTest(0xA4)) {
        camSunSetTunes5(0x518, 0x50, 0x51, 0x52, 0x53);
    }
    camSunAddSoundPair(0x131, 0x520);
    camSunAddSoundPair(0x130, 0x520);
    eneMakeEx(NULL, ENE_DEMO_KINOKO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
}
