#include "tlb.h"

extern ConnectData mafuu321PlacesEne0FAParam20[];
extern ConnectData mafuu321PlacesEne0FAParam21[];
extern ConnectData mafuu321PlacesEne0FAParam22[];
extern ConnectData mafuu321PlacesEne0FAParam23[];
extern ConnectData mafuu321PlacesEne0FAParam24[];
extern Tlb99Data mafuu321PlacesNormalEne0F3Param[];
extern ExitData mafuu321PlacesNormalEne0F9Param[];
ConnectData mafuu321PlacesEne0FAParam20[] = {
    { 20.0f, 62, 1, 100.0f, 85.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData mafuu321PlacesEne0FAParam21[] = {
    { 20.0f, 62, 1, 1760.0f, 330.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData mafuu321PlacesEne0FAParam22[] = {
    { 20.0f, 63, 1, 70.0f, 95.0f, 0.0f, 0.0f, 0x0000, 1, 0 },
};
ConnectData mafuu321PlacesEne0FAParam23[] = {
    { 20.0f, 63, 1, 2670.0f, 55.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData mafuu321PlacesEne0FAParam24[] = {
    { 20.0f, 64, 1, 75.0f, -5.0f, 0.0f, 0.0f, 0x0000, 1, 0 },
};
Tlb99Data mafuu321PlacesNormalEne0F3Param[] = {
    { 2, 1, { 0, 1, -1, -1 }, 168 },
};
ExitData mafuu321PlacesNormalEne0F9Param[] = {
    { 40.0f, 2, 1, -1, 0 },
};
EneDef mafuu321Places[] = {
    { 1, 125, 30, 0, ENE_TANU_WALL_FLAGS_0, 0, 0 },
    { 2, 296, 189, 0, ENE_NDRAM, 2, 1 },
    { 2, 433, 184, 0, ENE_NDRAM, 2, 1 },
    { 1, 1681, 16, 0, ENE_NDRAM, 2, 1 },
    { 1, 1753, 17, 0, ENE_NDRAM, 2, 1 },
    { 4, 404, 197, 0, ENE_NDRAM, 2, 1 },
    { 4, 474, 198, 0, ENE_NDRAM, 2, 1 },
    { 4, 1240, 35, 0, ENE_NDRAM, 2, 1 },
    { 4, 1307, 36, 0, ENE_NDRAM, 2, 1 },
    { 5, 586, 183, 0, ENE_NDRAM, 2, 0 },
    { 5, 524, 191, 0, ENE_NDRAM, 2, 0 },
    { 5, 249, 256, 0, ENE_NDRAM, 2, 0 },
    { 2, 204, 288, 0, ENE_GIROTIN, 2, 80 },
    { 3, 184, 172, 0, ENE_GIROTIN, 2, 80 },
    { 4, 206, 340, 0, ENE_GIROTIN, 2, 80 },
    { 4, 257, 332, 0, ENE_GIROTIN, 2, 80 },
    { 4, 449, 301, 0, ENE_GIROTIN, 2, 80 },
    { 4, 805, 222, 0, ENE_GIROTIN, 2, 80 },
    { 4, 1268, 135, 0, ENE_GIROTIN, 2, 80 },
    { 4, 1334, 138, 0, ENE_GIROTIN, 2, 80 },
    { 3, 373, 115, 0, ENE_CONNECT, 2, (u32) &mafuu321PlacesEne0FAParam20 },
    { 4, 2, 306, 0, ENE_CONNECT, 2, (u32) &mafuu321PlacesEne0FAParam21 },
    { 1, 1900, 35, 0, ENE_CONNECT, 2, (u32) &mafuu321PlacesEne0FAParam22 },
    { 5, 50, 300, 0, ENE_CONNECT, 2, (u32) &mafuu321PlacesEne0FAParam23 },
    { 2, 826, 240, 0, ENE_CONNECT, 2, (u32) &mafuu321PlacesEne0FAParam24 },
    { 1, 607, 125, 0, ENE_BPOLE, 2, 0 },
    { 1, 1015, 133, 0, ENE_BPOLE, 2, 0 },
    { 1, 1040, 80, 0, ENE_BPOLE, 2, 0 },
    { 1, 936, 195, 0, ENE_BPOLE, 2, 0 },
    { 0, 0, 14, 0, ENE_MAGMA_FLAGS_3, 2, 14 },
};
EneDef mafuu321PlacesNormal[] = {
    { 1, 125, 30, 0, ENE_EXIT_ESCAPE, 2, (u32) &mafuu321PlacesNormalEne0F9Param },
    { 1, 1005, 213, 0, ENE_SASROCK, 2, 160 },
    { 1, 1005, 253, 0, ENE_SASROCK, 2, 160 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_HAMAG_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_TEAR, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_CLOUD, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_SMOKE, 0, 0 },
    { 0, 0, 0, 0, ENE_FIREBALL, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 1, 320, 70, 0, ENE_SABURO, 2, 0 },
    { 1, 365, 92, 0, ENE_MONOEYE, 2, 1 },
    { 1, 365, 35, 0, ENE_ETSK_SH_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 450, 80, 0, ENE_SABURO, 2, 0 },
    { 1, 505, 95, 0, ENE_SABURO, 2, 1 },
    { 1, 550, 120, 0, ENE_SABURO, 2, 1 },
    { 1, 725, 158, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 2 },
    { 1, 725, 210, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 1, 765, 158, 0, ENE_ETSK_SH_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 805, 158, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 2 },
    { 1, 805, 215, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 1, 845, 158, 0, ENE_ETSK_SH_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 885, 160, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 2 },
    { 1, 885, 220, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 1, 925, 165, 0, ENE_ETSK_SH_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 2, 125, 240, 0, ENE_SABURO, 2, 0 },
    { 2, 175, 240, 0, ENE_SABURO, 2, 0 },
    { 2, 112, 215, 0, ENE_NUPPE, 2, 0 },
    { 2, 240, 217, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 2, 285, 240, 0, ENE_SABURO, 2, 1 },
    { 2, 500, 240, 0, ENE_SABURO, 2, 1 },
    { 2, 610, 227, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 2, 655, 265, 0, ENE_SABURO, 2, 1 },
    { 2, 665, 227, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 750, 110, 0, ENE_HELLBAT, 2, 2051 },
    { 1, 765, 95, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 790, 110, 0, ENE_HELLBAT, 2, 2051 },
    { 1, 810, 95, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 855, 95, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 940, 105, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 965, 105, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 1188, 100, 0, ENE_SABURO, 2, 1 },
    { 1, 1209, 80, 0, ENE_SABURO, 2, 1 },
    { 1, 1305, 90, 0, ENE_SABURO, 2, 1 },
    { 1, 1325, 90, 0, ENE_SABURO, 2, 1 },
    { 1, 1145, 50, 0, ENE_NUPPE, 2, 0 },
    { 1, 1400, 48, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 1400, 48, 0, ENE_ETSK_BETO_MAIN, 2, 1 },
    { 1, 1432, 70, 0, ENE_SABURO, 2, 0 },
    { 1, 1498, 105, 0, ENE_MONOEYE, 2, 1 },
    { 1, 1732, 65, 0, ENE_SABURO, 2, 0 },
    { 3, 70, 88, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 3, 70, 88, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 3, 120, 102, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 3, 120, 102, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 4, 348, 297, 0, ENE_MONOEYE, 2, 1 },
    { 4, 734, 160, 0, ENE_SABURO, 2, 0 },
    { 4, 1105, 85, 0, ENE_SABURO, 2, 1 },
    { 4, 1143, 85, 0, ENE_SABURO, 2, 1 },
    { 4, 227, 265, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 4, 569, 240, 0, ENE_SABURO, 2, 0 },
    { 4, 695, 165, 0, ENE_SABURO, 2, 0 },
    { 4, 774, 160, 0, ENE_SABURO, 2, 0 },
    { 4, 1035, 80, 0, ENE_NUPPE, 2, 0 },
    { 4, 1085, 55, 0, ENE_NUPPE, 2, 0 },
    { 4, 1420, 70, 0, ENE_HELLBAT, 2, 2560 },
    { 4, 1455, 105, 0, ENE_MONOEYE, 2, 1 },
    { 4, 1480, 65, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 5, 188, 290, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 5, 229, 307, 0, ENE_SABURO, 2, 0 },
    { 5, 311, 280, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 5, 418, 245, 0, ENE_NUPPE, 2, 0 },
    { 5, 566, 232, 0, ENE_SABURO, 2, 0 },
    { 5, 648, 205, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 5, 724, 185, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 5, 800, 160, 0, ENE_NUPPE, 2, 0 },
    { 5, 864, 152, 0, ENE_NUPPE, 2, 0 },
    { 5, 992, 120, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 0, 0, 0, 0, ENE_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_KOBAN, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR_MOJI, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR, 0, 0 },
    { 4, 1832, 80, 0, ENE_TANU_J, 2, (u32) &mafuu321PlacesNormalEne0F3Param },
    { 0, 0, 0, 0, ENE_EPRG_TANU, 0, 0 },
    { 1, 1570, 77, 0, ENE_TRANSFER, 2, 0 },
    { 2, 375, 203, 0, ENE_TRANSFER, 2, 0 },
};
EneDef mafuu321PlacesEvent[] = {
    { 0, 0, 0, 0, ENE_DEMO_KINOKO, 0, 0 },
    { 4, 1908, 52, 0, ENE_TANU_WALL_FLAGS_0, 0, 0 },
    { 1, 258, 32, 0, ENE_KINOKO, 2, 1 },
    { 1, 641, 123, 0, ENE_KINOKO, 2, 258 },
    { 1, 779, 184, 0, ENE_KINOKO, 2, 259 },
    { 1, 1247, 43, 0, ENE_KINOKO, 2, 4 },
    { 1, 1660, 94, 0, ENE_KINOKO, 2, 261 },
    { 1, 642, 192, 0, ENE_KINOKO, 2, 6 },
    { 1, 1715, 92, 0, ENE_KINOKO, 2, 519 },
    { 1, 995, 214, 0, ENE_KINOKO, 2, 8 },
    { 2, 187, 240, 0, ENE_KINOKO, 2, 9 },
    { 1, 778, 147, 0, ENE_KINOKO, 2, 522 },
    { 4, 1270, 79, 0, ENE_KINOKO, 2, 267 },
    { 4, 1783, 55, 0, ENE_KINOKO, 2, 12 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, 0, 0, 0, ENE_KINOKO_DISP, 0, 0 },
};

void mafuu321StageBuild(void) {
    if (swWorkTest(0xD) && !swWorkTest(0xA4)) {
        stageBuildBasic2(0x31, 1, 200.0f, 34.0f, 0.0f, (u32) mafuu321Places, 0x1E, (u32) mafuu321PlacesEvent, 0x12);
        PRINTF("イベント用セット OK\n");
    } else {
        stageBuildBasic2Atk150(0x31, 1, 200.0f, 34.0f, 0.0f, (u32) mafuu321Places, 0x1E, (u32) mafuu321PlacesNormal, 0x59);
    }
    camAiSetFar(800);
    camSunSetProgram(5);
    camAiAddLimit(1, 1, 0x744);
    camAiAddLimit(2, 1, 800);
    camAiAddLimit(3, 1, 0x14A);
    camAiAddLimit(4, -1, 60);
    if (!swWorkTest(0xD) || swWorkTest(0xA4)) {
        camSunSetTunes5(0x518, 0x50, 0x51, 0x52, 0x53);
    }
    sndPlay(0x521);
    eneMakeEx(NULL, ENE_DEMO_KINOKO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
}
