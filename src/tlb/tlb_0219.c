#include "tlb.h"

extern ConnectData castle24PlacesEne0FAParam0[];
extern ConnectData castle24PlacesEne0FAParam1[];
extern u32 castle24PlacesEne05CParam[];
ConnectData castle24PlacesEne0FAParam0[] = {
    { 60.0f, 52, 1, 170.0f, -180.0f, 0.0f, 0.0f, 0x0000, 0, 2 },
};
ConnectData castle24PlacesEne0FAParam1[] = {
    { 50.0f, 53, 1, 305.0f, -130.0f, 0.0f, 0.0f, 0x0000, 0, 2 },
};
u32 castle24PlacesEne05CParam[] = { 0x00000004, 0x000002AD, 0x00000064, 0x000001F4, 0x00000064 };
EneDef castle24Places[] = {
    { 1, 1720, 60, 0, ENE_CONNECT, 2, (u32) &castle24PlacesEne0FAParam0 },
    { 3, 1200, 110, 0, ENE_CONNECT, 2, (u32) &castle24PlacesEne0FAParam1 },
    { 1, 155, 226, 0, ENE_TRANSFER, 2, 0 },
    { 3, 1110, 220, 0, ENE_TRANSFER, 2, 0 },
    { 1, 290, 211, 0, ENE_ITEM_KOREZO_MAIN, 2, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_POT_FRAGMENT, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_TAKO_SUMI_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_TAKO_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_BOATSKEL_HEAD_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SIROBZ, 0, 0 },
    { 1, 1580, 195, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1500, 185, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1380, 175, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 4 },
    { 1, 1275, 185, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1190, 178, -70, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1170, 178, -70, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1150, 178, -70, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1130, 178, -70, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1130, 185, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1050, 185, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1000, 165, 0, ENE_EYEFROG_MAIN, 2, 2 },
    { 1, 975, 165, 0, ENE_EYEFROG_MAIN, 2, 2 },
    { 1, 870, 220, 0, ENE_ETSK_YADO_MAIN, 2, 1 },
    { 1, 735, 172, -55, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 690, 220, 0, ENE_ETSK_YADO_MAIN, 2, 1 },
    { 1, 485, 220, 0, ENE_ETSK_YADO_MAIN, 2, 1 },
    { 1, 535, 172, -55, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 280, 210, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 4 },
    { 1, 220, 220, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 4 },
    { 2, 445, 250, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 2, 480, 250, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 2, 600, 250, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 2, 660, 250, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 2, 690, 250, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 2, 710, 250, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 2, 810, 250, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 2, 815, 250, 0, ENE_ETSK_RHANI_MAIN_HOURS_ANY, 2, 1 },
    { 2, 910, 250, -40, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 2, 930, 250, -40, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 3, 180, 230, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 3, 310, 230, 0, ENE_ETSK_S_STONE_MAIN, 2, 1 },
    { 3, 450, 256, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 3, 450, 230, 0, ENE_ETSK_S_STONE_MAIN, 2, 1 },
    { 3, 530, 230, 0, ENE_ETSK_S_STONE_MAIN, 2, 1 },
    { 3, 600, 256, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 3, 610, 230, 0, ENE_ETSK_S_STONE_MAIN, 2, 1 },
    { 3, 760, 195, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 3, 810, 180, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 2, 1 },
    { 3, 830, 180, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 3, 910, 205, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 3, 990, 195, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 4, 300, 235, 0, ENE_EYEFROG_MAIN, 2, 2 },
    { 4, 360, 235, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 4, 370, 235, 0, ENE_EYEFROG_MAIN, 2, 2 },
    { 4, 430, 265, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 4, 435, 235, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 4, 460, 235, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 4, 470, 235, 0, ENE_EYEFROG_MAIN, 2, 2 },
    { 4, 535, 235, 0, ENE_ETSK_RHANI_MAIN_HOURS_ANY, 2, 1 },
    { 0, 0, 100, 0, ENE_WS_MASTER2, 3, (u32) &castle24PlacesEne05CParam },
    { 0, 3004, 200, -231, ENE_WS_SOUND2, 3, (u32) &castle24PlacesEne05CParam },
    { 0, 2200, 180, -700, ENE_WATER_FLAGS_3, 2, 10 },
    { 0, 3050, 225, -200, ENE_WATER_FLAGS_2, 2, 11 },
    { 1, 233, 221, 0, ENE_IMP_OBJ_BALL, 2, 0 },
};

void castle24StageBuild(void) {
    stageBuildBasic(0x1F, 1, 350.0f, 165.0f, 0.0f, (u32) castle24Places, 0x45);
    soundPlayAfterOne(0xA5);
    sndSetReverb(1, 15);
    camAiSetFar(600);
    gTimeOfDay = 12 << 16;
    camSunSetClockStop(1);
    camSunSetSkyOn(0);
    camSunSetProgram(0xE);
    camSunSetColor(255, 255, 255);
    camSunSetFogColor(0, 0, 0);
    gGameState.fade = 0;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
}
