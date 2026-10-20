#include "tlb.h"

extern ConnectData stage36PlacesEne0FAParam1[];
extern ConnectData stage36PlacesEne0FAParam2[];
extern ExitData stage36Places2Ene0F9Param[];
ConnectData stage36PlacesEne0FAParam1[] = {
    { 20.0f, 66, 1, 100.0f, -180.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData stage36PlacesEne0FAParam2[] = {
    { 20.0f, 67, 1, 100.0f, 10.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ExitData stage36Places2Ene0F9Param[] = {
    { 40.0f, 2, 4, -1, 0 },
};
EneDef stage36Places[] = {
    { 1, 90, 65, 0, ENE_TANU_WALL_FLAGS_0, 0, 0 },
    { 2, 64, 218, 0, ENE_CONNECT, 2, (u32) &stage36PlacesEne0FAParam1 },
    { 1, 3860, 0, 0, ENE_CONNECT, 2, (u32) &stage36PlacesEne0FAParam2 },
    { 1, 3455, 36, 0, ENE_BPOLE, 2, 0 },
    { 1, 3410, 70, 0, ENE_BPOLE, 2, 0 },
    { 1, 634, 92, 0, ENE_GIROTIN, 2, 80 },
    { 1, 972, 94, 0, ENE_GIROTIN, 2, 80 },
    { 1, 1017, 94, 0, ENE_GIROTIN, 2, 80 },
    { 1, 1062, 92, 0, ENE_GIROTIN, 2, 80 },
    { 1, 1242, 95, 0, ENE_GIROTIN, 2, 80 },
    { 1, 1330, 105, 5, ENE_GIROTIN, 2, 262224 },
    { 1, 1330, 105, -5, ENE_GIROTIN, 2, 262224 },
    { 1, 1448, 111, 0, ENE_GIROTIN, 2, 80 },
    { 1, 1485, 111, 0, ENE_GIROTIN, 2, 80 },
    { 1, 2715, 134, 0, ENE_GIROTIN, 2, 80 },
    { 1, 2756, 130, 0, ENE_GIROTIN, 2, 80 },
    { 1, 2829, 130, 0, ENE_GIROTIN, 2, 80 },
    { 1, 3512, 70, 0, ENE_GIROTIN, 2, 80 },
    { 2, 478, 226, 0, ENE_GIROTIN, 2, 80 },
    { 2, 426, 236, 0, ENE_GIROTIN, 2, 80 },
    { 2, 374, 243, 0, ENE_GIROTIN, 2, 80 },
};
EneDef stage36Places2[] = {
    { 1, 90, 65, 0, ENE_EXIT_ESCAPE, 2, (u32) &stage36Places2Ene0F9Param },
    { 1, 2392, 105, 0, ENE_ITEM_KOREZO_MAIN, 2, 0 },
    { 0, 0, 0, 0, ENE_POT_FRAGMENT, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_TEAR, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_CLOUD, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_SMOKE, 0, 0 },
    { 0, 0, 0, 0, ENE_FIREBALL, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_ONO, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_KOZ, 0, 0 },
    { 0, 0, 0, 0, ENE_NORU_MARK, 0, 0 },
    { 1, 521, 20, 0, ENE_ETSK_BETO_MAIN, 2, 3 },
    { 1, 678, 15, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 782, 23, 0, ENE_ETSK_BETO_MAIN, 2, 3 },
    { 1, 919, 14, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 1265, 31, 0, ENE_ETSK_BETO_MAIN, 2, 3 },
    { 1, 1561, 13, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 1978, 118, 0, ENE_ETSK_BETO_MAIN, 2, 8 },
    { 1, 2147, 118, 0, ENE_ETSK_BETO_MAIN, 2, 8 },
    { 1, 2689, 68, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 2786, 58, 0, ENE_ETSK_BETO_MAIN, 2, 3 },
    { 1, 2961, 52, 0, ENE_ETSK_BETO_MAIN, 2, 2 },
    { 2, 319, 180, 0, ENE_ETSK_BETO_MAIN, 2, 2 },
    { 2, 599, 101, 0, ENE_ETSK_BETO_MAIN, 2, 3 },
    { 2, 724, 77, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 200, 65, -10, ENE_DEADHAND, 2, 0 },
    { 1, 230, 65, 17, ENE_DEADHAND, 2, 0 },
    { 1, 260, 68, 5, ENE_DEADHAND, 2, 0 },
    { 0, -1133, 72, 191, ENE_DEADHAND, 2, 2949121 },
    { 0, -1175, 76, 189, ENE_DEADHAND, 2, 11796481 },
    { 1, 303, 100, 0, ENE_HELLBAT, 2, 2048 },
    { 1, 320, 90, 0, ENE_HELLBAT, 2, 2048 },
    { 1, 430, 80, 0, ENE_HELLBAT, 2, 2048 },
    { 1, 480, 50, 0, ENE_HELLBAT, 2, 2048 },
    { 1, 580, 20, 0, ENE_MOUJA, 2, 1 },
    { 1, 755, 15, 0, ENE_MOUJA, 2, 0 },
    { 1, 990, 35, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 1038, 35, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 1197, 50, 0, ENE_SABURO, 2, 1 },
    { 1, 868, 20, 0, ENE_MOUJA, 2, 0 },
    { 1, 1143, 15, -10, ENE_DEADHAND, 2, 0 },
    { 1, 1154, 10, 10, ENE_DEADHAND, 2, 0 },
    { 1, 1171, 10, -15, ENE_DEADHAND, 2, 0 },
    { 1, 1325, 30, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 7 },
    { 1, 1465, 47, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 1632, 40, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 1678, 50, 0, ENE_HELLBAT, 2, 2048 },
    { 1, 1700, 80, 0, ENE_HELLBAT, 2, 2048 },
    { 1, 1760, 107, 0, ENE_MONOEYE, 2, 1 },
    { 1, 1846, 80, 15, ENE_DEADHAND, 2, 0 },
    { 1, 1858, 85, -10, ENE_DEADHAND, 2, 0 },
    { 1, 1873, 85, 10, ENE_DEADHAND, 2, 0 },
    { 1, 1921, 105, 0, ENE_MOUJA, 2, 1 },
    { 1, 2056, 114, -18, ENE_DEADHAND, 2, 0 },
    { 1, 2070, 115, 15, ENE_DEADHAND, 2, 0 },
    { 1, 2082, 115, 18, ENE_DEADHAND, 2, 0 },
    { 1, 2085, 115, -17, ENE_DEADHAND, 2, 0 },
    { 1, 2112, 120, 0, ENE_MOUJA, 2, 0 },
    { 1, 2203, 118, 5, ENE_DEADHAND, 2, 0 },
    { 1, 2221, 120, 0, ENE_POT, 2, 0 },
    { 1, 2240, 115, -18, ENE_DEADHAND, 2, 0 },
    { 1, 2240, 115, 18, ENE_DEADHAND, 2, 0 },
    { 1, 2276, 150, 0, ENE_SABURO, 2, 1 },
    { 1, 2290, 140, 0, ENE_SABURO, 2, 1 },
    { 1, 2590, 92, 0, ENE_MOUJA, 2, 0 },
    { 1, 2733, 115, 0, ENE_MONOEYE, 2, 1 },
    { 1, 2900, 95, 0, ENE_SABURO, 2, 0 },
    { 1, 2963, 95, 0, ENE_SABURO, 2, 0 },
    { 1, 2926, 95, 0, ENE_SABURO, 2, 0 },
    { 1, 3136, 45, 0, ENE_SENSYA_KOZURE, 0, 0 },
    { 1, 3173, 61, 0, ENE_HELLBAT, 2, 2048 },
    { 1, 3204, 48, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 3238, 59, 0, ENE_HELLBAT, 2, 1024 },
    { 1, 3250, 35, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 3288, 23, 0, ENE_HELLBAT, 2, 2048 },
    { 1, 3306, 38, 0, ENE_HELLBAT, 2, 2048 },
    { 1, 3346, 12, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 3370, -5, 0, ENE_HELLBAT, 2, 1024 },
    { 1, 3402, 5, 0, ENE_HELLBAT, 2, 1280 },
    { 2, 242, 185, 0, ENE_NUPPE, 2, 0 },
    { 2, 625, 100, 0, ENE_NUPPE, 2, 0 },
    { 2, 400, 170, 0, ENE_NUPPE, 2, 0 },
    { 2, 520, 134, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 2, 725, 77, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 3641, 5, 0, ENE_SABURO, 2, 1 },
    { 1, 3716, 12, 0, ENE_SABURO, 2, 1 },
    { 1, 3630, -15, 0, ENE_MOUJA, 2, 0 },
    { 1, 3684, -15, 0, ENE_MOUJA, 2, 0 },
};
EneDef stage36KinokoPlaces[] = {
    { 0, 0, 0, 0, ENE_DEMO_KINOKO, 0, 0 },
    { 1, 229, 68, 0, ENE_KINOKO, 2, 1 },
    { 1, 635, 57, 0, ENE_KINOKO, 2, 258 },
    { 1, 923, 55, 0, ENE_KINOKO, 2, 259 },
    { 1, 1061, 61, 0, ENE_KINOKO, 2, 260 },
    { 1, 1366, 56, 0, ENE_KINOKO, 2, 261 },
    { 1, 2184, 117, 0, ENE_KINOKO, 2, 6 },
    { 1, 3569, -14, 0, ENE_KINOKO, 2, 7 },
    { 1, 465, 88, 0, ENE_KINOKO, 2, 264 },
    { 1, 3452, 117, 0, ENE_KINOKO, 2, 265 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, 0, 0, 0, ENE_KINOKO_DISP, 0, 0 },
};
EneDef stage36NightPlaces[] = {
    { 0, 0, 0, 0, ENE_FIREBALL, 0, 0 },
    { 1, 200, 65, -10, ENE_DEADHAND, 2, 0 },
    { 1, 230, 65, 17, ENE_DEADHAND, 2, 0 },
    { 1, 260, 68, 5, ENE_DEADHAND, 2, 0 },
    { 0, -1133, 72, 191, ENE_DEADHAND, 2, 2949121 },
    { 0, -1175, 76, 189, ENE_DEADHAND, 2, 11796481 },
    { 1, 300, 85, 0, ENE_HELLBAT, 2, 0 },
    { 1, 315, 80, 0, ENE_HELLBAT, 2, 0 },
    { 1, 400, 120, 0, ENE_GYUKI, 2, 0 },
    { 0, 0, 0, 0, ENE_BIG_BOMB, 0, 0 },
    { 1, 500, 60, 0, ENE_MONOEYE, 2, 1 },
    { 1, 580, 60, 0, ENE_MONOEYE, 2, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_TEAR, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_CLOUD, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_SMOKE, 0, 0 },
    { 1, 740, 60, 0, ENE_SABURO, 2, 0 },
    { 1, 820, 60, 0, ENE_SABURO, 2, 1 },
    { 1, 900, 15, 0, ENE_NUPPE, 2, 0 },
    { 1, 1100, 20, 0, ENE_MOUJA, 2, 0 },
    { 1, 1190, 20, 0, ENE_MOUJA, 2, 1 },
    { 1, 1560, 20, 0, ENE_MOUJA, 2, 65536 },
    { 1, 1750, 60, 0, ENE_MOUJA, 2, 65537 },
    { 0, 0, 0, 0, ENE_MOUJA_ONO, 0, 0 },
    { 1, 1350, 30, 0, ENE_SENSYA_KOZURE, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_KOZ, 0, 0 },
    { 0, 0, 0, 0, ENE_NORU_MARK, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_NECKBITE_TAMA_MAIN, 0, 0 },
    { 1, 1970, 123, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 1, 2065, 120, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
};

void stage36Build(void) {
    if (swWorkTest(0xD) && !swWorkTest(0xA4)) {
        stageBuildBasic2(0x24, 1, 150.0f, 65.0f, 0.0f, (u32) stage36Places, 0x15, (u32) stage36KinokoPlaces, 0xE);
        PRINTF("イベント用セット OK\n");
    } else {
        stageBuildBasic2Atk150(0x24, 1, 150.0f, 65.0f, 0.0f, (u32) stage36Places, 0x15, (u32) stage36Places2, 0x59);
    }
    camAiSetFar(800);
    camAiAddLimit(2, -1, 0x73);
    camSunSetProgram(5);
    cambgMove(cambgCreate(gStageTask, 4, 800.0f), 0, 0x20);
    if (!swWorkTest(0xD) || swWorkTest(0xA4)) {
        camSunSetTunes5(0x518, 0x54, 0x55, 0x56, 0x57);
    }
    eneMakeEx(NULL, ENE_DEMO_KINOKO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
}

void stage36BuildNight(void) {
    stageBuildBasic(0x24, 1, 70.0f, 65.0f, 0.0f, (u32) stage36NightPlaces, 0x22);
    camAiSetFar(800);
    camSunSetProgram(5);
    cambgMove(cambgCreate(gStageTask, 4, 800.0f), 0, 0x20);
    camSunSetTunes5(0x518, 0x54, 0x55, 0x56, 0x57);
    sndPlay(0x521);
    gTimeOfDay = 20 << 16;
}
