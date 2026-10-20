#include "tlb.h"

extern ConnectData ryugu216PlacesEne0FAParam[];
extern Tlb99Data ryugu216PlacesEne0F3Param[];
ConnectData ryugu216PlacesEne0FAParam[] = {
    { 100.0f, 42, 1, 1080.0f, 95.0f, 0.0f, 0.0f, 0x82AD, 0, 2 },
};
Tlb99Data ryugu216PlacesEne0F3Param[] = {
    { 1, 0, { 1, 0, -1, -1 }, 161 },
};
EneDef ryugu216Places[] = {
    { 1, 95, -102, 0, ENE_CONNECT, 2, (u32) &ryugu216PlacesEne0FAParam },
    { 1, 160, 100, 0, ENE_TRANSFER, 2, 0 },
    { 1, 200, 90, 0, ENE_IDASHI, 2, 40 },
    { 1, 250, 30, 0, ENE_IMP_OBJ_CUBE, 2, 0 },
    { 1, 250, 50, 0, ENE_MANJI, 2, 0 },
    { 1, 270, 30, 0, ENE_SASROCK, 2, 0 },
    { 0, 0, 0, 0, ENE_MANJI_PIECE, 2, 0 },
    { 1, 290, 51, 0, ENE_MANJI, 2, 0 },
    { 1, 290, 71, 0, ENE_MANJI, 2, 0 },
    { 1, 290, 91, 0, ENE_MANJI, 2, 0 },
    { 1, 290, 111, 0, ENE_MANJI, 2, 0 },
    { 1, 310, 51, 0, ENE_SASROCK, 2, 0 },
    { 1, 310, 92, 0, ENE_IMP_OBJ_CUBE, 2, 0 },
    { 1, 310, 112, 0, ENE_MANJI, 2, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_SIROBZ, 0, 0 },
    { 1, 380, 53, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 430, 53, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 530, 74, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 545, 74, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 560, 74, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 795, 99, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 890, 93, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1010, 74, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1225, 74, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1245, 74, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1285, 5, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 0, 0, 16, 0, ENE_OBJ22_WATER_FLAGS_3, 2, 3 },
    { 1, 638, 63, 0, ENE_IDASHI_SLIDE, 2, 70 },
    { 1, 748, 54, 0, ENE_IDASHI, 2, 32 },
    { 1, 798, 44, 0, ENE_IDASHI, 2, 32 },
    { 1, 843, 60, 0, ENE_IDASHI, 2, 32 },
    { 1, 880, 65, 0, ENE_IDASHI_SLIDE, 2, 70 },
    { 1, 1103, 60, 0, ENE_IDASHI, 2, 32 },
    { 1, 1150, 60, 0, ENE_IDASHI, 2, 32 },
    { 0, 0, 0, 0, ENE_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_KOBAN, 0, 0 },
    { 0, 0, 0, 0, ENE_EPRG_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR_MOJI, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR, 0, 0 },
    { 1, 1392, 61, 0, ENE_TANU_J, 2, (u32) &ryugu216PlacesEne0F3Param },
};

void ryugu216StageBuild(void) {
    stageBuildBasic(0x17, 1, 230.0f, 30.0f, 0.0f, (u32) ryugu216Places, 0x2E);
    camSunSetTunes5(0x518, 0x3E, 0x3F, 0x40, 0x41);
    sndSetReverb(1, 15);
    camSunAddSoundPair(0x28D, 0x520);
    camSunAddSoundPair(0x12F, 0x520);
    camAiSetFar(800);
    camSunSetProgram(8);
}
