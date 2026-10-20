#include "tlb.h"

extern Tlb99Data mafuu324PlacesNormalEne0F3Param[];
extern ConnectData mafuu324PlacesEne0FAParam[];
Tlb99Data mafuu324PlacesNormalEne0F3Param[] = {
    { 2, 1, { 0, 2, -1, -1 }, 171 },
};
ConnectData mafuu324PlacesEne0FAParam[] = {
    { 20.0f, 61, 2, 790.0f, 225.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef mafuu324Places[] = {
    { 1, 45, -5, 0, ENE_CONNECT, 2, (u32) &mafuu324PlacesEne0FAParam },
    { 0, 0, -64, 0, ENE_MAGMA_FLAGS_3, 2, 15 },
};
EneDef mafuu324PlacesNormal[] = {
    { 0, 0, 0, 0, ENE_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_KOBAN, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR_MOJI, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR, 0, 0 },
    { 1, 1280, 9, 0, ENE_TANU_J, 2, (u32) &mafuu324PlacesNormalEne0F3Param },
    { 0, 0, 0, 0, ENE_EPRG_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_TEAR, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_CLOUD, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_SMOKE, 0, 0 },
    { 0, 0, 0, 0, ENE_FIREBALL, 0, 0 },
    { 0, 0, 0, 0, ENE_POT_FRAGMENT, 0, 0 },
    { 1, 192, -7, 0, ENE_POT, 2, 0 },
    { 1, 393, -23, 0, ENE_POT, 2, 0 },
    { 1, 535, -17, 0, ENE_POT, 2, 0 },
    { 1, 630, -17, 0, ENE_POT, 2, 0 },
    { 1, 705, 2, 0, ENE_POT, 2, 0 },
    { 1, 800, 2, 0, ENE_POT, 2, 0 },
    { 1, 919, 2, 0, ENE_POT, 2, 0 },
    { 1, 977, 11, 0, ENE_POT, 2, 0 },
    { 1, 1040, 21, 0, ENE_POT, 2, 0 },
    { 1, 1095, 31, 0, ENE_POT, 2, 0 },
    { 1, 280, -10, 0, ENE_CYCLOPS, 0, 0 },
    { 1, 574, -17, 0, ENE_CYCLOPS, 0, 0 },
    { 1, 763, 2, 0, ENE_CYCLOPS, 0, 0 },
    { 1, 1144, 31, 0, ENE_CYCLOPS, 0, 0 },
    { 1, 240, 10, 0, ENE_SABURO, 2, 0 },
    { 1, 745, 25, 0, ENE_SABURO, 2, 0 },
    { 1, 800, 25, 0, ENE_SABURO, 2, 0 },
    { 1, 200, 10, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 240, 10, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 330, 0, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 600, 0, 0, ENE_HELLBAT, 2, 1024 },
    { 1, 650, 0, 0, ENE_HELLBAT, 2, 1024 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 1, 482, -34, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
};
EneDef mafuu324PlacesEvent[] = {
    { 0, 0, 0, 0, ENE_DEMO_KINOKO, 0, 0 },
    { 1, 1353, 9, 0, ENE_TANU_WALL_FLAGS_0, 0, 0 },
    { 1, 437, 20, 0, ENE_KINOKO, 2, 285 },
    { 1, 761, 0, 0, ENE_KINOKO, 2, 30 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, 0, 0, 0, ENE_KINOKO_DISP, 0, 0 },
};

void mafuu324StageBuild(void) {
    if (swWorkTest(0xD) && !swWorkTest(0xA4)) {
        stageBuildBasic2(0x34, 1, 75.0f, -5.0f, 0.0f, (u32) mafuu324Places, 2, (u32) mafuu324PlacesEvent, 8);
        PRINTF("イベント用セット OK\n");
    } else {
        stageBuildBasic2Atk150(0x34, 1, 75.0f, -5.0f, 0.0f, (u32) mafuu324Places, 2, (u32) mafuu324PlacesNormal, 0x24);
    }
    camAiSetFar(800);
    camSunSetProgram(5);
    camAiAddLimit(1, -1, 0x5A);
    if (!swWorkTest(0xD) || swWorkTest(0xA4)) {
        camSunSetTunes5(0x518, 0x50, 0x51, 0x52, 0x53);
    }
    sndSetReverb(1, 15);
    camSunAddSoundPair(0x131, 0x520);
    camSunAddSoundPair(0x130, 0x520);
    eneMakeEx(NULL, ENE_DEMO_KINOKO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
}
