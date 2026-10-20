#include "tlb.h"

extern Tlb99Data ryugu233PlacesEne0F3Param[];
extern ConnectData ryugu233PlacesEne0FAParam[];
Tlb99Data ryugu233PlacesEne0F3Param[] = {
    { 1, 3, { 3, -1, -1, -1 }, 160 },
};
ConnectData ryugu233PlacesEne0FAParam[] = {
    { 40.0f, 48, 1, 2646.0f, -30.0f, 0.0f, 0.0f, 0x0000, 0, 2 },
};
EneDef ryugu233Places[] = {
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_SIROBZ, 0, 0 },
    { 1, 300, 2, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 2 },
    { 1, 350, 2, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 2 },
    { 1, 370, 2, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 2 },
    { 1, 400, 57, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 1, 440, 2, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 2 },
    { 1, 510, 7, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 560, 32, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 590, 7, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 640, -5, 0, ENE_ETSK_BETO_MAIN, 2, 8 },
    { 1, 660, 20, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 715, -5, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 740, -5, 0, ENE_ETSK_BETO_MAIN, 2, 1 },
    { 1, 750, -5, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 850, 2, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 1 },
    { 1, 20, -30, 0, ENE_TANU_WALL_FLAGS_0, 0, 0 },
    { 1, 85, -70, 0, ENE_CONNECT, 2, (u32) &ryugu233PlacesEne0FAParam },
    { 0, 0, 0, 0, ENE_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_KOBAN, 0, 0 },
    { 0, 0, 0, 0, ENE_EPRG_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR_MOJI, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR, 0, 0 },
    { 1, 1000, 60, 0, ENE_TANU_J, 2, (u32) &ryugu233PlacesEne0F3Param },
    { 0, 0, 0, 0, ENE_HOTARU, 2, 0 },
    { 0, 0, 1, 0, ENE_HOTARU_MASTER, 2, 0 },
    { 0, 0, -14, 0, ENE_OBJ22_WATER_FLAGS_3, 2, 3 },
};

void ryugu233StageBuild(void) {
    Obj *bg;

    stageBuildBasic(0x1C, 1, 200.0f, 25.0f, 0.0f, (u32) ryugu233Places, 0x20);
    camSunSetTunes2(0x48, 0x49);
    camAiSetFar(800);
    bg = cambgCreate(gStageTask, 3, 800.0f);
    cambgMove(bg, 0, -8);
    cambgScrollY(bg, 0);
}
