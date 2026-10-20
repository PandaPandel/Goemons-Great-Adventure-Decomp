#include "tlb.h"

extern ConnectData stage48PlacesEne0FAParam[];
extern Tlb99Data stage48Places2Ene0F3Param[];
ConnectData stage48PlacesEne0FAParam[] = {
    { 20.0f, 67, 1, 2030.0f, 10.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
Tlb99Data stage48Places2Ene0F3Param[] = {
    { 2, 4, { 4, -1, -1, -1 }, 201 },
};
EneDef stage48Places[] = {
    { 1, 45, 93, 0, ENE_CONNECT, 2, (u32) &stage48PlacesEne0FAParam },
    { 1, 466, 112, -9, ENE_GIROTIN, 2, 80 },
    { 1, 528, 103, 0, ENE_GIROTIN, 2, 80 },
    { 1, 570, 97, 0, ENE_GIROTIN, 2, 80 },
    { 1, 992, 92, 0, ENE_GIROTIN, 2, 80 },
    { 1, 1038, 93, 0, ENE_GIROTIN, 2, 80 },
    { 1, 1084, 89, 0, ENE_GIROTIN, 2, 80 },
    { 1, 1433, 47, 0, ENE_GIROTIN, 2, 80 },
};
EneDef stage48Places2[] = {
    { 0, 0, 0, 0, ENE_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_KOBAN, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR_MOJI, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR, 0, 0 },
    { 1, 1680, -35, 0, ENE_TANU_J, 2, (u32) &stage48Places2Ene0F3Param },
    { 0, 0, 0, 0, ENE_EPRG_TANU, 0, 0 },
    { 1, 875, 96, 0, ENE_ITEM_OOIRI_MAIN, 2, 407 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_MANJI_PIECE, 2, 0 },
    { 1, 855, 92, 0, ENE_MANJI, 2, 0 },
    { 1, 855, 112, 0, ENE_MANJI, 2, 0 },
    { 1, 855, 132, 0, ENE_MANJI, 2, 0 },
    { 1, 894, 92, 0, ENE_MANJI, 2, 0 },
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
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 1, 277, 50, 0, ENE_SENSYA_KOZURE, 0, 0 },
    { 1, 320, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 2 },
    { 1, 360, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 2 },
    { 1, 378, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 2 },
    { 1, 401, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 2 },
    { 1, 422, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 2 },
    { 1, 320, 50, 0, ENE_ETSK_BETO_MAIN, 2, 3 },
    { 1, 360, 50, 0, ENE_ETSK_BETO_MAIN, 2, 3 },
    { 1, 378, 50, 0, ENE_ETSK_BETO_MAIN, 2, 3 },
    { 1, 401, 50, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 422, 50, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 545, 32, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 547, 45, 0, ENE_SABURO, 2, 0 },
    { 1, 605, 41, 0, ENE_SABURO, 2, 0 },
    { 1, 505, 36, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 596, 28, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 724, 81, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 1338, -36, 0, ENE_ETSK_BETO_MAIN, 2, 2 },
    { 1, 1381, -36, 0, ENE_ETSK_BETO_MAIN, 2, 2 },
    { 1, 677, 120, 0, ENE_SABURO, 2, 1 },
    { 1, 799, 68, 0, ENE_POT, 2, 0 },
    { 1, 732, 15, 0, ENE_MOUJA, 2, 0 },
    { 1, 812, 15, 0, ENE_MOUJA, 2, 0 },
    { 1, 782, 68, 18, ENE_DEADHAND, 2, 0 },
    { 1, 800, 68, -10, ENE_DEADHAND, 2, 0 },
    { 1, 816, 68, 15, ENE_DEADHAND, 2, 0 },
    { 1, 852, 12, 18, ENE_DEADHAND, 2, 0 },
    { 1, 872, 12, -10, ENE_DEADHAND, 2, 0 },
    { 1, 886, 12, 5, ENE_DEADHAND, 2, 0 },
    { 1, 903, 10, 11, ENE_DEADHAND, 2, 0 },
    { 1, 891, 60, 0, ENE_SABURO, 2, 1 },
    { 1, 894, 41, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 944, 14, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1059, 22, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1060, 50, 0, ENE_SABURO, 2, 1 },
    { 1, 1030, -15, 0, ENE_SABURO, 2, 1 },
    { 1, 1050, -15, 0, ENE_SABURO, 2, 1 },
    { 1, 1154, 20, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 1203, 33, 0, ENE_HELLBAT, 2, 2560 },
    { 1, 1228, 25, 0, ENE_HELLBAT, 2, 2560 },
    { 1, 1290, 8, 0, ENE_SABURO, 2, 0 },
    { 1, 1334, -10, 0, ENE_SABURO, 2, 0 },
    { 1, 13900, -15, 0, ENE_SABURO, 2, 0 },
    { 1, 1509, -35, 11, ENE_DEADHAND, 2, 0 },
    { 1, 1526, -35, -8, ENE_DEADHAND, 2, 0 },
    { 1, 1539, -35, 15, ENE_DEADHAND, 2, 0 },
    { 1, 1560, -35, 11, ENE_DEADHAND, 2, 0 },
};
EneDef stage48KinokoPlaces[] = {
    { 0, 0, 0, 0, ENE_DEMO_KINOKO, 0, 0 },
    { 1, 1763, -32, 0, ENE_TANU_WALL_FLAGS_0, 0, 0 },
    { 1, 463, 71, 0, ENE_KINOKO, 2, 283 },
    { 1, 926, 119, 0, ENE_KINOKO, 2, 284 },
    { 1, 1038, 57, 0, ENE_KINOKO, 2, 285 },
    { 1, 1678, -33, 0, ENE_KINOKO, 2, 30 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, 0, 0, 0, ENE_KINOKO_DISP, 0, 0 },
};

void stage48Build(void) {
    if (swWorkTest(0xD) && !swWorkTest(0xA4)) {
        stageBuildBasic2(0x30, 1, 100.0f, 90.0f, 0.0f, (u32) stage48Places, 8, (u32) stage48KinokoPlaces, 0xA);
        PRINTF("イベント用セット OK\n");
    } else {
        stageBuildBasic2Atk150(0x30, 1, 100.0f, 90.0f, 0.0f, (u32) stage48Places, 8, (u32) stage48Places2, 0x4B);
    }
    camAiSetFar(800);
    camSunSetProgram(5);
    camAiAddLimit(1, -1, 0x5A);
    if (!swWorkTest(0xD) || swWorkTest(0xA4)) {
        camSunSetTunes5(0x518, 0x54, 0x55, 0x56, 0x57);
    }
    sndPlay(0x521);
    eneMakeEx(NULL, ENE_DEMO_KINOKO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
}
