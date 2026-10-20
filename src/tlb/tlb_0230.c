#include "tlb.h"

extern ConnectData stage47PlacesEne0FAParam0[];
extern ConnectData stage47PlacesEne0FAParam1[];
ConnectData stage47PlacesEne0FAParam0[] = {
    { 20.0f, 65, 1, 3830.0f, 0.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData stage47PlacesEne0FAParam1[] = {
    { 20.0f, 68, 1, 100.0f, 90.0f, 0.0f, 0.0f, 0x0521, 0, 0 },
};
EneDef stage47Places[] = {
    { 1, 70, 10, 0, ENE_CONNECT, 2, (u32) &stage47PlacesEne0FAParam0 },
    { 1, 2060, 10, 0, ENE_CONNECT, 2, (u32) &stage47PlacesEne0FAParam1 },
    { 0, -187, -30, 253, ENE_MAGMA_FLAGS_3, 2, 14 },
};
EneDef stage47Places2[] = {
    { 0, 0, 0, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 0, 0 },
    { 0, 0, 0, 0, ENE_A3KP, 2, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_TEAR, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_CLOUD, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_SMOKE, 0, 0 },
    { 0, 0, 0, 0, ENE_FIREBALL, 0, 0 },
    { 1, 200, 0, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 200, 0, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 280, -5, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 280, -5, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 393, 20, 0, ENE_HELLBAT, 2, 2048 },
    { 1, 418, 25, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 450, 20, 0, ENE_HELLBAT, 2, 2048 },
    { 1, 494, -50, 0, ENE_A3KP_GEN, 2, 524288 },
    { 1, 530, 30, 0, ENE_ETSK_LANTERN_MAIN, 2, 1 },
    { 1, 565, 0, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 609, 5, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 670, -50, 0, ENE_A3KP_GEN, 2, 524288 },
    { 1, 734, 5, 0, ENE_A3KP_BEARD, 2, 262147 },
    { 1, 790, 5, 0, ENE_A3KP_BEARD, 2, 262147 },
    { 1, 857, 60, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 1, 955, 60, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 1, 900, -5, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 1047, 10, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 4 },
    { 1, 1050, 40, 0, ENE_SABURO, 2, 0 },
    { 1, 1100, 40, 0, ENE_SABURO, 2, 0 },
    { 1, 1150, 40, 0, ENE_SABURO, 2, 0 },
    { 1, 991, 11, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1044, 8, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1097, 5, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1150, 3, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1210, 2, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1188, 5, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 4 },
    { 1, 1250, -50, 0, ENE_A3KP_GEN, 2, 524288 },
    { 1, 1495, 8, 0, ENE_ETSK_BETO_MAIN, 2, 2 },
    { 1, 1621, 5, 0, ENE_ETSK_BETO_MAIN, 2, 2 },
    { 1, 1730, 5, 0, ENE_ETSK_BETO_MAIN, 2, 2 },
    { 1, 1896, -7, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 1421, 40, 0, ENE_SABURO, 2, 1 },
    { 1, 1491, 35, 0, ENE_SABURO, 2, 1 },
    { 1, 1588, 37, 0, ENE_SABURO, 2, 1 },
    { 1, 1620, 45, 0, ENE_SABURO, 2, 1 },
    { 1, 1653, 28, 0, ENE_SABURO, 2, 1 },
    { 1, 1758, 27, 0, ENE_SABURO, 2, 1 },
    { 1, 1356, 20, 0, ENE_HELLBAT, 2, 2048 },
    { 1, 1383, 20, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 1449, 20, 0, ENE_HELLBAT, 2, 2560 },
    { 1, 1528, 22, 0, ENE_HELLBAT, 2, 1024 },
    { 1, 1698, 22, 0, ENE_HELLBAT, 2, 1024 },
    { 1, 1789, 20, 0, ENE_HELLBAT, 2, 1024 },
    { 1, 1816, 25, 0, ENE_HELLBAT, 2, 1024 },
    { 1, 1920, -5, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
};
EneDef stage47KinokoPlaces[] = {
    { 0, 0, 0, 0, ENE_DEMO_KINOKO, 0, 0 },
    { 1, 243, 29, 0, ENE_KINOKO, 2, 280 },
    { 1, 896, 27, 0, ENE_KINOKO, 2, 281 },
    { 1, 1917, -9, 0, ENE_KINOKO, 2, 26 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, 0, 0, 0, ENE_KINOKO_DISP, 0, 0 },
};

void stage47Build(void) {
    if (swWorkTest(0xD) && !swWorkTest(0xA4)) {
        stageBuildBasic2(0x2F, 1, 100.0f, 10.0f, 0.0f, (u32) stage47Places, 3, (u32) stage47KinokoPlaces, 8);
        PRINTF("イベント用セット OK\n");
    } else {
        stageBuildBasic2Atk150(0x2F, 1, 100.0f, 10.0f, 0.0f, (u32) stage47Places, 3, (u32) stage47Places2, 0x34);
    }
    camAiSetFar(800);
    camSunSetProgram(8);
    if (!swWorkTest(0xD) || swWorkTest(0xA4)) {
        camSunSetTunes5(0x518, 0x54, 0x55, 0x56, 0x57);
    }
    sndSetReverb(1, 15);
    camSunAddSoundPair(0x131, 0x520);
    camSunAddSoundPair(0x130, 0x520);
    eneMakeEx(NULL, ENE_DEMO_KINOKO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
}
