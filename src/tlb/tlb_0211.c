#include "tlb.h"

extern u32 ryugu22PlacesEne05CParam15[];
extern u32 ryugu22PlacesEne05CParam18[];
extern u32 ryugu22PlacesEne05CParam23[];
extern Tlb99Data ryugu22PlacesNormalEne0F3Param[];
extern ConnectData ryugu22PlacesNormalEne0FAParam[];
extern ExitData ryugu22PlacesNormalEne0F9Param[];
u32 ryugu22PlacesEne05CParam15[] = { 0x00000004, 0x000002AD, 0x00000032, 0x000001F4, 0x00000000 };
u32 ryugu22PlacesEne05CParam18[] = { 0x00000004, 0x00000286, 0x00000000, 0x000001F4, 0x00000014 };
u32 ryugu22PlacesEne05CParam23[] = { 0x00000004, 0x00000287, 0x00000000, 0x000001F4, 0x00000014 };
Tlb99Data ryugu22PlacesNormalEne0F3Param[] = {
    { 1, 1, { 2, -1, -1, -1 }, 159 },
};
ConnectData ryugu22PlacesNormalEne0FAParam[] = {
    { 20.0f, 192, 0, 10.0f, 20.0f, -2.0f, 0.0f, 0x0000, 0, 0 },
};
ExitData ryugu22PlacesNormalEne0F9Param[] = {
    { 40.0f, 1, 1, -1, 0 },
};
EneDef ryugu22Places[] = {
    { 1, 149, 167, 0, ENE_TANU_WALL_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_HOTARU, 2, 0 },
    { 0, 0, 1, 0, ENE_HOTARU_MASTER, 2, 0 },
    { 0, 194, 100, 378, ENE_OBJ22_SISI, 2, 830 },
    { 0, -144, 200, 857, ENE_OBJ22_KERO, 2, 457 },
    { 0, -194, 220, -174, ENE_OBJ22_ELEVATOR22, 2, 140 },
    { 0, -143, 256, 180, ENE_OBJ22_ELEVATOR22, 2, 170 },
    { 0, 365, 159, -3, ENE_OBJ22_GEAR1, 2, 0 },
    { 0, 377, 131, 196, ENE_OBJ22_GEAR2, 2, 1 },
    { 0, 377, 131, -208, ENE_OBJ22_GEAR2, 2, 0 },
    { 0, 33, 138, -253, ENE_OBJ22_GEAR3, 2, 0 },
    { 1, 3405, -13, 0, ENE_OBJ22_HASU, 2, 50 },
    { 1, 3254, -13, 0, ENE_OBJ22_HASU, 2, 100 },
    { 1, 3044, -13, 0, ENE_OBJ22_HASU, 2, 40 },
    { 1, 3129, -13, 0, ENE_OBJ22_HASU, 2, 80 },
    { 0, 0, 100, 0, ENE_WS_MASTER2, 3, (u32) &ryugu22PlacesEne05CParam15 },
    { 0, 78, 138, 427, ENE_WS_SOUND2, 3, (u32) &ryugu22PlacesEne05CParam15 },
    { 0, -32, 330, 325, ENE_WS_SOUND2, 3, (u32) &ryugu22PlacesEne05CParam15 },
    { 0, 0, 120, 0, ENE_WS_MASTER2, 3, (u32) &ryugu22PlacesEne05CParam18 },
    { 0, 213, 387, -21, ENE_WS_SOUND2, 3, (u32) &ryugu22PlacesEne05CParam18 },
    { 0, 213, 244, -21, ENE_WS_SOUND2, 3, (u32) &ryugu22PlacesEne05CParam18 },
    { 0, -199, 373, -13, ENE_WS_SOUND2, 3, (u32) &ryugu22PlacesEne05CParam18 },
    { 0, -199, 68, -13, ENE_WS_SOUND2, 3, (u32) &ryugu22PlacesEne05CParam18 },
    { 0, 0, 140, 0, ENE_WS_MASTER2, 3, (u32) &ryugu22PlacesEne05CParam23 },
    { 0, 26, 390, -220, ENE_WS_SOUND2, 3, (u32) &ryugu22PlacesEne05CParam23 },
    { 0, 26, 157, -220, ENE_WS_SOUND2, 3, (u32) &ryugu22PlacesEne05CParam23 },
    { 0, -4, 418, 0, ENE_OBJ22_WATER_FLAGS_3, 2, 1 },
    { 0, -140, 200, 854, ENE_OBJ22_WATER_FLAGS_1, 2, 2 },
    { 0, 0, -13, 0, ENE_OBJ22_WATER_FLAGS_3, 2, 3 },
    { 0, -37, 360, 319, ENE_OBJ22_WATER_FLAGS_2, 2, 4 },
    { 0, -59, 194, 620, ENE_OBJ22_WATER_FLAGS_2, 2, 5 },
    { 0, 213, 418, -21, ENE_OBJ22_WATER_FLAGS_3, 2, 16777222 },
    { 0, -199, 418, -13, ENE_OBJ22_WATER_FLAGS_3, 2, 50331654 },
    { 0, 26, 418, -220, ENE_OBJ22_WATER_FLAGS_3, 2, 33554438 },
    { 0, -119, 309, 458, ENE_OBJ22_FALLS22, 2, 927 },
    { 0, -25, 379, 327, ENE_OBJ22_FALLS22_V, 2, 1012 },
    { 0, 84, 138, 411, ENE_OBJ22_FALLS22_R, 2, 384 },
    { 1, 1540, -15, 0, ENE_SPIKE, 2, 8126664 },
    { 1, 1580, -15, 0, ENE_SPIKE, 2, 8126664 },
    { 1, 1620, -15, 0, ENE_SPIKE, 2, 8126664 },
    { 3, 770, -15, 0, ENE_SPIKE, 2, 8126664 },
    { 3, 810, -15, 0, ENE_SPIKE, 2, 8126664 },
    { 3, 850, -15, 0, ENE_SPIKE, 2, 8126664 },
    { 2, 1593, 415, 0, ENE_SPIKE, 2, 8126614 },
    { 2, 1645, 415, 0, ENE_SPIKE, 2, 8126564 },
    { 2, 1892, 415, 0, ENE_SPIKE, 2, 8126564 },
    { 2, 1934, 415, 0, ENE_SPIKE, 2, 8126564 },
};
EneDef ryugu22PlacesNormal[] = {
    { 6, 211, 410, 0, ENE_CONNECT, 2, (u32) &ryugu22PlacesNormalEne0FAParam },
    { 1, 149, 167, 0, ENE_EXIT_ESCAPE, 2, (u32) &ryugu22PlacesNormalEne0F9Param },
    { 5, 112, 244, 0, ENE_ITEM_OOIRI_MAIN, 2, 404 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 4, 336, 89, 0, ENE_IMP_OBJ_CUBE, 2, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_TAKO_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_HAMAG_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_POT_FRAGMENT, 0, 0 },
    { 0, 0, 0, 0, ENE_BOATSKEL_HEAD_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SIROBZ, 0, 0 },
    { 2, 380, 320, 0, ENE_ITEM_KOREZO_MAIN, 2, 0 },
    { 4, 170, 138, 0, ENE_ITEM_KOREZO_MAIN, 2, 0 },
    { 1, 290, 167, 25, ENE_BOATSKEL_HEAD_MAIN, 2, 0 },
    { 1, 310, 167, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 1, 385, 167, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 1, 415, 167, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 1, 500, 205, 0, ENE_SIRO_J_HOURS_ANY, 2, 2 },
    { 1, 580, 165, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 600, 165, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 680, 165, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 580, 165, 0, ENE_ETSK_BETO_MAIN, 2, 6 },
    { 1, 600, 165, 0, ENE_ETSK_BETO_MAIN, 2, 8 },
    { 1, 680, 165, 0, ENE_ETSK_BETO_MAIN, 2, 7 },
    { 1, 700, 195, 0, ENE_SIRO_J_HOURS_ANY, 2, 0 },
    { 1, 860, 228, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 1, 875, 178, -5, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 5 },
    { 1, 915, 178, -5, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 5 },
    { 1, 1020, 195, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1040, 170, 0, ENE_POT, 2, 0 },
    { 1, 1335, 177, 0, ENE_ETSK_TAKO_MAIN, 2, 3 },
    { 1, 1435, 162, 0, ENE_ETSK_TAKO_MAIN, 2, 3 },
    { 1, 1480, 142, 0, ENE_ETSK_TAKO_MAIN, 2, 3 },
    { 1, 1470, -2, 0, ENE_HAMAG_J, 2, 1 },
    { 1, 1520, -21, 0, ENE_HAMAG_J, 2, 1 },
    { 3, 550, 189, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 8 },
    { 3, 450, 194, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 8 },
    { 3, 420, 196, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 8 },
    { 3, 305, 261, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 3, 270, 261, -5, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 3, 200, 261, -5, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 3, 175, 261, -5, ENE_ETSK_KT_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 3, 160, 261, 0, ENE_ETSK_BETO_MAIN, 2, 8 },
    { 3, 70, 302, -5, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 3, 125, 261, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 4 },
    { 2, 105, 310, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 4 },
    { 2, 165, 375, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 2, 220, 320, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 2, 275, 320, 0, ENE_ETSK_RHANI_MAIN_HOURS_ANY, 2, 1 },
    { 2, 305, 320, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 2, 480, 301, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 2, 520, 321, 0, ENE_SIRO_J_HOURS_5_19, 2, 1 },
    { 2, 510, 331, 0, ENE_ETSK_LANTERN_MAIN, 2, 1 },
    { 2, 570, 309, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 2, 640, 301, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 2, 765, 350, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 4 },
    { 2, 840, 350, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 5 },
    { 2, 910, 347, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1885, 151, 0, ENE_ETSK_BETO_MAIN, 2, 4 },
    { 1, 2090, 186, 0, ENE_ETSK_LANTERN_MAIN, 2, 1 },
    { 1, 2200, 134, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 5 },
    { 1, 2220, 134, 0, ENE_ETSK_BETO_MAIN, 2, 7 },
    { 1, 2285, 134, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2455, 151, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 2540, 161, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 2735, 107, 0, ENE_ETSK_BETO_MAIN, 2, 6 },
    { 1, 2810, -12, 0, ENE_HAMAG_J, 2, 1 },
    { 1, 2950, 24, 0, ENE_ETSK_BETO_MAIN, 2, 2 },
    { 1, 2975, -76, 0, ENE_HAMAG_J, 2, 1 },
    { 1, 3043, -17, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 10 },
    { 1, 3250, 19, 0, ENE_SIRO_J_HOURS_ANY, 2, 2 },
    { 1, 3330, 4, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 3450, 19, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 3585, 25, 0, ENE_POT, 2, 0 },
    { 4, 480, 44, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 4, 430, 60, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 4, 355, 79, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 4, 270, 145, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 4, 240, 125, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_5_19, 2, 5 },
    { 4, 120, 138, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 4, 80, 149, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 5, 170, 189, 0, ENE_ETSK_BETO_MAIN, 2, 1 },
    { 5, 210, 244, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 5, 220, 189, 0, ENE_ETSK_BETO_MAIN, 2, 1 },
    { 5, 240, 209, 0, ENE_SIRO_J_HOURS_5_19, 2, 1 },
    { 5, 340, 205, 0, ENE_ETSK_RHANI_MAIN_HOURS_ANY, 2, 1 },
    { 5, 440, 210, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 5, 470, 210, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 5, 580, 361, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 5, 595, 361, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 5, 775, 347, 0, ENE_ETSK_TAKO_MAIN, 2, 3 },
    { 5, 825, 327, 0, ENE_ETSK_TAKO_MAIN, 2, 3 },
    { 5, 840, 263, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 5, 885, 263, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 5, 920, 463, 0, ENE_SIRO_J_HOURS_5_19, 2, 1 },
    { 5, 960, 492, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 5, 1050, 437, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 5, 1100, 437, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 5, 1225, 466, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 2, 1110, 420, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 2, 1125, 420, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 2, 1140, 420, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 2, 1155, 420, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 2, 1170, 420, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 2, 1240, 402, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 2, 1255, 423, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 2, 1300, 244, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 2, 1470, 475, -60, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 2, 1415, 462, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 2, 1490, 462, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 2, 1515, 462, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 2, 1620, 455, 0, ENE_ETSK_BETO_MAIN, 2, 4 },
    { 2, 1730, 440, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 5 },
    { 2, 1830, 440, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 2, 1870, 440, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 2, 1740, 450, -60, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 2, 1860, 455, -60, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 2, 1915, 455, 0, ENE_ETSK_BETO_MAIN, 2, 7 },
    { 0, 0, 0, 0, ENE_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_KOBAN, 0, 0 },
    { 0, 0, 0, 0, ENE_EPRG_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR_MOJI, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR, 0, 0 },
    { 2, 2080, 482, 0, ENE_TANU_J, 2, (u32) &ryugu22PlacesNormalEne0F3Param },
};
EneDef ryugu22PlacesEvent[] = {
    { 6, 214, 389, 0, ENE_TANU_WALL_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_OBISU_EFFECT, 0, 0 },
    { 0, 0, 0, 0, ENE_OBISU_RUN, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_OBISU_RUN, 0, 0 },
    { 0, 0, 0, 0, ENE_OBISU_ATARI, 0, 0 },
};

void ryugu22StageBuild(void) {
    Obj *bg;

    if (swWorkTest(7) && !swWorkTest(0x9B)) {
        PRINTF("init_st046: おびすまるイベント用セットします\n");
        stageBuildBasic2(4, 1, 428.0f, 167.0f, 0.0f, (u32) ryugu22Places, 0x2F, (u32) ryugu22PlacesEvent, 5);
    } else {
        stageBuildBasic2(4, 1, 210.0f, 170.0f, 0.0f, (u32) ryugu22Places, 0x2F, (u32) ryugu22PlacesNormal, 0x80);
    }
    camAiSetFar(800);
    camAiAddLimit(1, -1, 200);
    camAiAddLimit(6, 1, 170);
    bg = cambgCreate(gStageTask, 3, 800.0f);
    cambgMove(bg, 0, -8);
    cambgScrollY(bg, 0);
    if (swWorkTest(7) && !swWorkTest(0x9B)) {
        sndPlay(0x8E);
    } else {
        camSunSetTunes5(0x518, 0x44, 0x45, 0x46, 0x47);
    }
    sndPlay(0x521);
    eneMakeEx(NULL, ENE_DEMO_OBISU_RUN, 2, 2150.0f, 482.0f, 0.0f, 0, NULL);
}
