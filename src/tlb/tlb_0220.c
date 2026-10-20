#include "tlb.h"

extern ConnectData castleBoss25PlacesEne0FAParam[];
ConnectData castleBoss25PlacesEne0FAParam[] = {
    { 40.0f, 53, 1, 4110.0f, -170.0f, 0.0f, 0.0f, 0x0000, 0, 2 },
};
EneDef castleBoss25Places[] = {
    { 1, 4400, -63, 0, ENE_CONNECT, 2, (u32) &castleBoss25PlacesEne0FAParam },
    { 1, 4150, -23, 0, ENE_ITEM_KOREZO_MAIN, 2, 0 },
    { 1, 4240, -39, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 4220, -39, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 2, 1 },
    { 1, 4170, -23, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 2126, 460, 0, ENE_ITEM_OOIRI_MAIN, 2, 426 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 1, 500, -45, 0, ENE_SANBA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SANBA, 0, 0 },
    { 0, 0, 0, 0, ENE_SANBA_JELLYFISH, 0, 0 },
    { 0, 0, 0, 0, ENE_SANBA_TARAI, 0, 0 },
    { 0, 0, 0, 0, ENE_SANBA_MARUTA, 0, 0 },
    { 0, 0, 0, 0, ENE_SANBA_PLDEMO, 0, 0 },
    { 0, 0, 0, 0, ENE_SANBA_WATER, 0, 0 },
    { 0, 0, 0, 0, ENE_SANBA_CHASE, 0, 0 },
    { 0, 0, 0, 0, ENE_SANBA_IKADA, 0, 0 },
    { 0, 0, 0, 0, ENE_SANBA_WINCH, 0, 0 },
    { 0, 0, 0, 0, ENE_SANBA_SIBUKI, 0, 0 },
    { 0, 0, 0, 0, ENE_SANBA_RIPPLE, 0, 0 },
    { 0, 0, 0, 0, ENE_SANBA_MARBL, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_3, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_GOLDNEKO_MAIN_FLAGS_3, 0, 0 },
};

void castleBoss25StageBuild(void) {
    frmDeleteKind(gFrmKindGpol);
    polInitGpolKind(250);
    stageBuildFlagged(0x23, 1, 4300.0f, -47.0f, 0.0f, (u32) castleBoss25Places, 0x18);
    camAiSetFar(800);
    camSunSetClockStop(0);
    camSunSetSkyOn(0);
    camSunSetProgram(0xE);
    gGameState.fade = 1;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
    gGameState.fadeStart = 0x30;
    gGameState.fadeTo[0] = 0x40;
    gGameState.fadeTo[1] = 0x40;
    gGameState.fadeTo[2] = 0x40;
    gGameState.fadeEnd = 0x80;
    camSunSetFogColor(16, 16, 16);
    camSunSetColor(0xD0, 0xD0, 0xFF);
}
