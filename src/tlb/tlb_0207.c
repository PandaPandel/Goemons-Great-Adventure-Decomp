#include "tlb.h"

extern ConnectData ryugu213PlacesEne0FAParam15[];
extern ConnectData ryugu213PlacesEne0FAParam16[];
extern ConnectData ryugu213PlacesEne0FAParam17[];
ConnectData ryugu213PlacesEne0FAParam15[] = {
    { 100.0f, 41, 1, 740.0f, -20.0f, 0.0f, 0.0f, 0x0000, 0, 4 },
};
ConnectData ryugu213PlacesEne0FAParam16[] = {
    { 100.0f, 41, 1, 1082.0f, -20.0f, 0.0f, 0.0f, 0x0000, 0, 4 },
};
ConnectData ryugu213PlacesEne0FAParam17[] = {
    { 100.0f, 45, 1, 95.0f, 5.0f, 0.0f, 0.0f, 0x0000, 0, 4 },
};
EneDef ryugu213Places[] = {
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_TAKO_SUMI_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ELEFISH_LIGHTNING, 0, 0 },
    { 1, 200, 35, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 450, 35, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 165, 5, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 575, 30, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 690, 50, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 830, 5, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 1035, 55, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 620, 70, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 880, 45, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 980, 75, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 100, 210, 0, ENE_CONNECT, 2, (u32) &ryugu213PlacesEne0FAParam15 },
    { 1, 390, 225, 0, ENE_CONNECT, 2, (u32) &ryugu213PlacesEne0FAParam16 },
    { 1, 1085, 225, 0, ENE_CONNECT, 2, (u32) &ryugu213PlacesEne0FAParam17 },
    { 0, 0, 10, 0, ENE_AQUA, 2, 0 },
    { 0, 0, 140, 0, ENE_OBJ22_WATER_FLAGS_3, 2, 3 },
    { 0, 0, 0, 0, ENE_DEMO_BIG_ELEFISH, 0, 0 },
    { 0, 0, 0, 0, ENE_BIG_ELE, 0, 0 },
    { 0, 0, 0, 0, ENE_ITEM_ATARI, 0, 0 },
    { 0, 0, 0, 0, ENE_EPRG_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_JIZOU_FILE_138, 0, 0 },
};

void ryugu213StageBuild(void) {
    stageBuildDive(0x14, 1, 90.0f, 130.0f, 0.0f, (u32) ryugu213Places, 0x1D);
    camAiSetFar(800);
    camSunSetProgram(4);
    camSunSetTunes2(0x43, 0x42);
    sndSetReverb(1, 15);
    camSunAddSoundPair(0x28D, 0x520);
    camSunAddSoundPair(0x12F, 0x520);
    eneMakeEx(NULL, ENE_DEMO_BIG_ELEFISH, 0, 0.0f, 0.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) 1);
}
