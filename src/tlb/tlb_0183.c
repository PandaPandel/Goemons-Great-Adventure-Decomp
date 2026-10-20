#include "tlb.h"

EneDef stage0183Places[] = {
    { 0, 0, 0, 0, ENE_UMA_NORU, 0, 0 },
    { 0, 0, 0, 0, ENE_UMA_HANI_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_UMA_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_NORU_MARK, 0, 0 },
    { 0, 0, 0, 0, ENE_NORU_MARK, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 1, 583, 69, 0, ENE_BPOLE, 2, 0 },
    { 1, 650, 89, 0, ENE_BPOLE, 2, 0 },
    { 1, 781, 135, 0, ENE_BPOLE, 2, 0 },
    { 1, 520, 25, 0, ENE_MANJI, 2, 0 },
    { 0, 0, 0, 0, ENE_MANJI_PIECE, 2, 0 },
    { 0, 628, 22, 1623, ENE_WATER_POOL_FLAGS_2, 2, 1 },
    { 0, 583, 39, 498, ENE_WATER_POOL_FLAGS_2, 2, 2818049 },
    { 0, -269, 56, -782, ENE_WATER_POOL_FLAGS_2, 2, 3997697 },
    { 0, -519, 32, -1659, ENE_WATER_POOL_FLAGS_2, 2, 2293761 },
    { 0, 69, 86, -274, ENE_WATER_POOL_FLAGS_1, 2, 41025538 },
    { 0, -21, 64, -401, ENE_WATER_POOL_FLAGS_1, 2, 40108034 },
    { 0, 412, 197, 1735, ENE_WATER_SPLASH, 2, 16777219 },
    { 0, -16, 248, -155, ENE_WATER_SPLASH, 2, 29949956 },
    { 0, 69, 86, -274, ENE_WATER_SPLASH, 2, 41025541 },
    { 0, 0, 0, 0, ENE_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_KOBAN, 0, 0 },
    { 1, 5840, 100, 0, ENE_TANU_J, 2, -16711424 },
    { 1, 680, 25, 0, ENE_UMA_SET_2F_HOURS_ANY, 2, 0 },
    { 0, -380, 110, -60, ENE_WATER_POOL_FLAGS_2, 2, 1 },
    { 1, 735, 20, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
};

void stageBuild0183(void) {
    stageBuildBasic(1, 1, 550.0f, 25.0f, 0.0f, (u32) stage0183Places, 0x1B);
    camAiSetFar(600);
    cambgCreate(gStageTask, 1, 800.0f);
    gTimeOfDay = 12 << 16;
    camSunRefreshHold(1);
}
