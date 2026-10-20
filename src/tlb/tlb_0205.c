#include "tlb.h"

extern ConnectData ryugu211PlacesEne0FAParam[];
extern ExitData ryugu211PlacesEne0F9Param[];
ConnectData ryugu211PlacesEne0FAParam[] = {
    { 40.0f, 41, 1, 270.0f, 4.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ExitData ryugu211PlacesEne0F9Param[] = {
    { 40.0f, 1, 0, -1, 0 },
};
EneDef ryugu211Places[] = {
    { 1, 1536, 147, 0, ENE_CONNECT, 2, (u32) &ryugu211PlacesEne0FAParam },
    { 1, 833, 158, 0, ENE_TRANSFER, 2, 0 },
    { 1, 1310, 26, 0, ENE_ITEM_KOREZO_MAIN, 2, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_SIROBZ, 0, 0 },
    { 1, 360, 170, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 390, 170, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 450, 170, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 480, 170, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 450, 170, 0, ENE_ETSK_BETO_MAIN, 2, 1 },
    { 1, 480, 170, 0, ENE_ETSK_BETO_MAIN, 2, 1 },
    { 1, 500, 200, 0, ENE_SIRO_J_HOURS_ANY, 2, 0 },
    { 1, 550, 170, -30, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 600, 210, 0, ENE_SIRO_J_HOURS_ANY, 2, 2 },
    { 1, 680, 170, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 720, 170, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 640, 170, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 680, 170, 0, ENE_ETSK_BETO_MAIN, 2, 4 },
    { 1, 720, 170, 0, ENE_ETSK_BETO_MAIN, 2, 4 },
    { 1, 785, 196, 0, ENE_SIRO_J_HOURS_ANY, 2, 0 },
    { 1, 810, 156, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 905, 170, 0, ENE_SIRO_J_HOURS_ANY, 2, 0 },
    { 1, 920, 140, 0, ENE_ETSK_BETO_MAIN, 2, 1 },
    { 1, 945, 140, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1100, 187, 0, ENE_ETSK_LANTERN_MAIN, 2, 1 },
    { 1, 1195, 125, 0, ENE_ETSK_KONAKI_MAIN_HOURS_19_5, 2, 1 },
    { 1, 1225, 125, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1245, 125, 0, ENE_ETSK_BETO_MAIN, 2, 6 },
    { 1, 1260, 125, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1285, 160, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1350, 180, 0, ENE_SIRO_J_HOURS_5_19, 2, 1 },
    { 1, 1420, 140, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1430, 185, 0, ENE_ETSK_LANTERN_MAIN, 2, 1 },
    { 1, 150, 170, 0, ENE_TANU_WALL_FLAGS_0, 0, 0 },
    { 1, 150, 170, 0, ENE_EXIT_ESCAPE, 2, (u32) &ryugu211PlacesEne0F9Param },
    { 1, 1509, 210, 0, ENE_TANU_WALL_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_HOTARU, 2, 0 },
    { 0, 0, 1, 0, ENE_HOTARU_MASTER, 2, 0 },
    { 1, 1030, 140, 0, ENE_IDASHI, 2, 32 },
    { 1, 1083, 123, 0, ENE_IDASHI, 2, 32 },
    { 1, 1136, 133, 0, ENE_IDASHI, 2, 32 },
    { 1, 1314, 112, 0, ENE_IDASHI, 2, 32 },
    { 1, 1365, 130, 0, ENE_IDASHI, 2, 32 },
};

void ryugu211StageBuild(void) {
    Obj *bg;

    stageBuildBasic(0x12, 1, 210.0f, 170.0f, 0.0f, (u32) ryugu211Places, 0x2F);
    camSunSetTunes5(0x518, 0x3E, 0x3F, 0x40, 0x41);
    sndPlay(0x521);
    camAiSetFar(800);
    camAiAddLimit(1, 1, 0x5C3);
    bg = cambgCreate(gStageTask, 3, 800.0f);
    cambgMove(bg, 0, -8);
    cambgScrollY(bg, 0);
}
