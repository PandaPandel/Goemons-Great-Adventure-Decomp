#include "tlb.h"

extern ConnectData ryugu231PlacesEne0FAParam[];
extern ExitData ryugu231PlacesEne0F9Param[];
ConnectData ryugu231PlacesEne0FAParam[] = {
    { 30.0f, 48, 1, 170.0f, -60.0f, 0.0f, 0.0f, 0x0000, 0, 2 },
};
ExitData ryugu231PlacesEne0F9Param[] = {
    { 40.0f, 1, 3, -1, 0 },
};
EneDef ryugu231Places[] = {
    { 1, 900, 0, 0, ENE_TANU_WALL_FLAGS_0, 0, 0 },
    { 1, 860, -75, 0, ENE_CONNECT, 2, (u32) &ryugu231PlacesEne0FAParam },
    { 1, 937, -76, 0, ENE_TANU_WALL_FLAGS_0, 0, 0 },
    { 1, 150, 2, 0, ENE_EXIT_ESCAPE, 2, (u32) &ryugu231PlacesEne0F9Param },
    { 1, 565, 7, 0, ENE_TRANSFER, 2, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 0, 0 },
    { 1, 340, 2, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 440, 2, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 470, 2, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 340, 2, 0, ENE_ETSK_BETO_MAIN, 2, 1 },
    { 1, 440, 2, 0, ENE_ETSK_BETO_MAIN, 2, 1 },
    { 1, 470, 2, 0, ENE_ETSK_BETO_MAIN, 2, 1 },
    { 1, 600, 7, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 5 },
    { 1, 640, 7, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 5 },
    { 1, 680, 62, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 1, 750, -5, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 760, -5, 0, ENE_ETSK_RHANI_MAIN_HOURS_ANY, 2, 1 },
    { 0, 0, 0, 0, ENE_HOTARU, 2, 0 },
    { 0, 0, 1, 0, ENE_HOTARU_MASTER, 2, 0 },
    { 0, 0, -28, 0, ENE_OBJ22_WATER_FLAGS_3, 2, 3 },
};

void ryugu231StageBuild(void) {
    Obj *bg;

    stageBuildBasic(0x19, 1, 210.0f, 5.0f, 0.0f, (u32) ryugu231Places, 0x19);
    camSunSetTunes2(0x48, 0x49);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 200);
    camAiAddLimit(1, 1, 0x352);
    bg = cambgCreate(gStageTask, 3, 800.0f);
    cambgMove(bg, 0, -8);
    cambgScrollY(bg, 0);
}
