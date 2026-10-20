#include "tlb.h"

EneDef stg0187Places[] = {
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_WANYU_FIRE_MAIN, 0, 0 },
    { 1, 300, 67, 0, ENE_ETSK_WANYU_MAIN, 0, 0 },
    { 1, 400, 20, 0, ENE_ETSK_SHIBAG_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_HIPEYE_UNKO_MAIN, 0, 0 },
    { 1, 500, 50, 0, ENE_ETSK_HIPEYE_MAIN, 2, 1 },
    { 1, 600, 50, 0, ENE_ETSK_KIJIM_MAIN, 2, 1 },
    { 0, 0, 0, 0, ENE_ELEFISH_LIGHTNING, 0, 0 },
    { 1, 1300, 30, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1390, 40, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1460, 30, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 0, 628, 2, 1623, ENE_WATER_POOL_FLAGS_2, 2, 1 },
    { 0, 583, 39, 498, ENE_WATER_POOL_FLAGS_2, 2, 2818049 },
    { 0, -269, 56, -782, ENE_WATER_POOL_FLAGS_2, 2, 3997697 },
    { 0, -519, 32, -1659, ENE_WATER_POOL_FLAGS_2, 2, 2293761 },
    { 0, 69, 86, -274, ENE_WATER_POOL_FLAGS_1, 2, 41025538 },
    { 0, -21, 64, -401, ENE_WATER_POOL_FLAGS_1, 2, 40108034 },
    { 0, 412, 197, 1735, ENE_WATER_SPLASH, 2, 16777219 },
    { 0, -6, 280, -145, ENE_WATER_SPLASH, 2, 29949956 },
    { 0, 69, 86, -274, ENE_WATER_SPLASH, 2, 41025541 },
    { 0, 0, 0, 0, ENE_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_KOBAN, 0, 0 },
    { 1, 5840, 100, 0, ENE_TANU_J, 2, -16711424 },
};

void stg0187Build(void) {
    stageBuildBasic(1, 1, 50.0f, 20.0f, 0.0f, (u32) stg0187Places, 0x1A);
    camAiSetFar(400);
    cambgCreate(gStageTask, 1, 800.0f);
}
