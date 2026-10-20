#include "tlb.h"

extern Tlb99Data stage37Places2Ene0F3Param[];
extern ConnectData stage37PlacesEne0FAParam[];
extern KkMasterData stage37PlacesEne081Param3[];
extern KkMasterData stage37PlacesEne081Param4[];
Tlb99Data stage37Places2Ene0F3Param[] = {
    { 2, 4, { 5, -1, -1, -1 }, 169 },
};
ConnectData stage37PlacesEne0FAParam[] = {
    { 20.0f, 65, 2, 110.0f, 205.0f, 0.0f, 0.0f, 0x0521, 0, 0 },
};
KkMasterData stage37PlacesEne081Param3[] = {
    { 1, { -903, -269, -1311 } },
};
KkMasterData stage37PlacesEne081Param4[] = {
    { 0, { 333, -4, -493 } },
};
EneDef stage37Places[] = {
    { 1, 47, -180, 0, ENE_CONNECT, 2, (u32) &stage37PlacesEne0FAParam },
    { 0, 538, 60, -127, ENE_MAGMA_FLAGS_3, 2, 13 },
    { 0, 0, 0, 0, ENE_KKOMA, 0, 0 },
    { 0, -61, 185, -821, ENE_KKMASTER, 3, (u32) &stage37PlacesEne081Param3 },
    { 0, 1292, 667, 1172, ENE_KKMASTER, 3, (u32) &stage37PlacesEne081Param4 },
    { 1, 3542, 404, 0, ENE_GIM_NBOARD, 2, 85 },
    { 1, 3555, 404, 0, ENE_GIM_NBOARD, 2, 85 },
    { 1, 3720, 454, 0, ENE_GIM_NBOARD, 2, 85 },
    { 1, 3733, 454, 0, ENE_GIM_NBOARD, 2, 85 },
    { 1, 1960, 29, 0, ENE_GIM_NBOARD, 2, 85 },
    { 1, 1973, 29, 0, ENE_GIM_NBOARD, 2, 85 },
    { 1, 2142, 34, 0, ENE_GIM_NBOARD, 2, 85 },
    { 1, 2155, 34, 0, ENE_GIM_NBOARD, 2, 85 },
    { 1, 2465, 115, 0, ENE_GIM_NBOARD, 2, 85 },
    { 1, 2478, 115, 0, ENE_GIM_NBOARD, 2, 85 },
    { 1, 2930, 240, 0, ENE_GIM_NBOARD, 2, 85 },
    { 1, 2943, 240, 0, ENE_GIM_NBOARD, 2, 85 },
};
EneDef stage37Places2[] = {
    { 0, 0, 0, 0, ENE_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_KOBAN, 0, 0 },
    { 1, 4635, 574, 0, ENE_TANU_J, 2, (u32) &stage37Places2Ene0F3Param },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR_MOJI, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR, 0, 0 },
    { 0, 0, 0, 0, ENE_EPRG_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_GIM_FBOARD, 0, 0 },
    { 0, 0, 0, 0, ENE_GIM_NBOARD, 0, 0 },
    { 1, 1825, 45, 0, ENE_ITEM_KOREZO_MAIN, 2, 0 },
    { 1, 206, -198, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 260, -198, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_POT_FRAGMENT, 0, 0 },
    { 1, 1690, 49, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 514, -192, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 585, -192, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 662, -189, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 738, -189, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 811, -173, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 885, -154, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 961, -133, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1037, -113, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1113, -92, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1188, -72, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1266, -51, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1339, -31, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1416, -12, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1491, 7, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1566, 27, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1912, 53, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2061, 56, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2212, 67, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2286, 87, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2363, 107, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2437, 127, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2512, 147, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2589, 167, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2663, 187, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2738, 208, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2815, 229, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2891, 249, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2966, 269, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3042, 289, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3118, 310, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3194, 330, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3267, 350, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3346, 371, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3416, 390, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3494, 411, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3570, 431, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3645, 451, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3795, 491, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3868, 511, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3948, 532, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 4020, 551, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 4133, 573, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 4190, 573, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
};
EneDef stage37KinokoPlaces[] = {
    { 0, 0, 0, 0, ENE_DEMO_KINOKO, 0, 0 },
    { 1, 4855, 554, 0, ENE_TANU_WALL_FLAGS_0, 0, 0 },
    { 1, 368, -202, 0, ENE_KINOKO, 2, 10 },
    { 1, 735, -192, 0, ENE_KINOKO, 2, 11 },
    { 1, 883, -157, 0, ENE_KINOKO, 2, 12 },
    { 1, 1033, -117, 0, ENE_KINOKO, 2, 13 },
    { 1, 1189, -76, 0, ENE_KINOKO, 2, 14 },
    { 1, 1419, -13, 0, ENE_KINOKO, 2, 15 },
    { 1, 1871, 51, 0, ENE_KINOKO, 2, 16 },
    { 1, 2102, 54, 0, ENE_KINOKO, 2, 17 },
    { 1, 2393, 116, 0, ENE_KINOKO, 2, 18 },
    { 1, 2625, 173, 0, ENE_KINOKO, 2, 19 },
    { 1, 3076, 296, 0, ENE_KINOKO, 2, 20 },
    { 1, 3604, 438, 0, ENE_KINOKO, 2, 21 },
    { 1, 4021, 549, 0, ENE_KINOKO, 2, 22 },
    { 1, 4631, 575, 0, ENE_KINOKO, 2, 23 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, 0, 0, 0, ENE_KINOKO_DISP, 0, 0 },
};

void stage37Build(void) {
    if (swWorkTest(0xD) && !swWorkTest(0xA4)) {
        stageBuildBasic2Atk150(0x25, 1, 100.0f, -180.0f, 0.0f, (u32) stage37Places, 0x11, (u32) stage37KinokoPlaces, 0x14);
        PRINTF("イベント用セット OK\n");
    } else {
        stageBuildBasic2Atk150(0x25, 1, 100.0f, -180.0f, 0.0f, (u32) stage37Places, 0x11, (u32) stage37Places2, 0x3B);
    }
    camAiSetFar(800);
    camSunSetProgram(8);
    camAiAddLimit(1, -1, 0x6E);
    if (!swWorkTest(0xD) || swWorkTest(0xA4)) {
        camSunSetTunes5(0x518, 0x54, 0x55, 0x56, 0x57);
    }
    sndSetReverb(1, 15);
    camSunAddSoundPair(0x131, 0x520);
    camSunAddSoundPair(0x130, 0x520);
    eneMakeEx(NULL, ENE_DEMO_KINOKO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
}
