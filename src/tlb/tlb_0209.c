#include "tlb.h"

extern ConnectData ryugu215PlacesEne0FAParam[];
extern Tlb99Data ryugu215PlacesEne0F3Param[];
ConnectData ryugu215PlacesEne0FAParam[] = {
    { 15.0f, 43, 1, 642.0f, 140.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
Tlb99Data ryugu215PlacesEne0F3Param[] = {
    { 1, 0, { 1, 4, -1, -1 }, 158 },
};
EneDef ryugu215Places[] = {
    { 1, 210, 36, 0, ENE_CONNECT, 2, (u32) &ryugu215PlacesEne0FAParam },
    { 1, 210, 57, 0, ENE_CONNECT, 2, (u32) &ryugu215PlacesEne0FAParam },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_HAMAG_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SIROBZ, 0, 0 },
    { 1, 394, -75, 0, ENE_HAMAG_J, 2, 1 },
    { 1, 425, 25, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 449, -51, 0, ENE_HAMAG_J, 2, 1 },
    { 1, 480, 49, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 505, 25, 0, ENE_ETSK_BETO_MAIN, 2, 4 },
    { 1, 560, 71, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 2 },
    { 1, 580, 71, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 2 },
    { 1, 700, 105, 0, ENE_ETSK_LANTERN_MAIN, 2, 1 },
    { 1, 845, 25, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 950, -20, 0, ENE_HAMAG_J, 2, 1 },
    { 1, 1010, -20, 0, ENE_HAMAG_J, 2, 1 },
    { 1, 1060, -20, 0, ENE_HAMAG_J, 2, 1 },
    { 1, 1120, 71, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1145, 71, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1220, 87, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1230, 112, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 0, 0, 10, 0, ENE_OBJ22_WATER_FLAGS_3, 2, 3 },
    { 1, 652, 63, 0, ENE_IDASHI_SLIDE, 2, 70 },
    { 1, 760, 56, 0, ENE_IDASHI, 2, 32 },
    { 1, 878, 55, 0, ENE_IDASHI, 2, 32 },
    { 1, 930, 48, 0, ENE_IDASHI, 2, 32 },
    { 1, 983, 54, 0, ENE_IDASHI, 2, 32 },
    { 1, 1033, 61, 0, ENE_IDASHI, 2, 32 },
    { 1, 1198, 60, 0, ENE_IDASHI_SLIDE, 2, 70 },
    { 0, 0, 0, 0, ENE_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_KOBAN, 0, 0 },
    { 0, 0, 0, 0, ENE_EPRG_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR_MOJI, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR, 0, 0 },
    { 1, 1373, 59, 0, ENE_TANU_J, 2, (u32) &ryugu215PlacesEne0F3Param },
};

void ryugu215StageBuild(void) {
    stageBuildBasic(0x16, 1, 200.0f, 30.0f, 0.0f, (u32) ryugu215Places, 0x28);
    camSunSetTunes5(0x518, 0x3E, 0x3F, 0x40, 0x41);
    sndSetReverb(1, 15);
    camSunAddSoundPair(0x28D, 0x520);
    camSunAddSoundPair(0x12F, 0x520);
    camAiSetFar(800);
    camSunSetProgram(8);
}
