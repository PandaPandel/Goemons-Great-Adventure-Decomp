#include "tlb.h"

extern ConnectData aisle1PlacesEne0FAParam0[];
extern ConnectData aisle1PlacesEne0FAParam1[];
extern ConnectData aisle2PlacesEne0FAParam0[];
extern ConnectData aisle2PlacesEne0FAParam1[];
extern ConnectData aisle3PlacesEne0FAParam0[];
extern ConnectData aisle3PlacesEne0FAParam1[];
extern ConnectData aisle4PlacesEne0FAParam0[];
extern ConnectData aisle4PlacesEne0FAParam1[];
/* aisle1Init-aisle3Init set their two doors' tune (8 or 0) by gGameWork.tunePart. */
ConnectData aisle1PlacesEne0FAParam0[] = {
    { 30.0f, 107, 1, 3364.0f, -69.0f, 0.0f, 0.0f, 0x0008, 0, 0 },
};
ConnectData aisle1PlacesEne0FAParam1[] = {
    { 30.0f, 108, 1, 247.0f, 10.0f, 0.0f, 0.0f, 0x0008, 0, 0 },
};
EneDef aisle1Places[] = {
    { 1, 78, 15, 0, ENE_CONNECT, 2, (u32) &aisle1PlacesEne0FAParam0 },
    { 1, 338, 15, 0, ENE_CONNECT, 2, (u32) &aisle1PlacesEne0FAParam1 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 1, 165, 15, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 185, 15, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 205, 15, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 2, 1 },
    { 1, 225, 15, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 245, 15, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
};
ConnectData aisle2PlacesEne0FAParam0[] = {
    { 30.0f, 114, 1, 770.0f, 76.0f, 0.0f, 0.0f, 0x0008, 0, 0 },
};
ConnectData aisle2PlacesEne0FAParam1[] = {
    { 30.0f, 115, 1, 68.0f, 40.0f, 0.0f, 0.0f, 0x0008, 0, 0 },
};
EneDef aisle2Places[] = {
    { 1, 78, 15, 0, ENE_CONNECT, 2, (u32) &aisle2PlacesEne0FAParam0 },
    { 1, 338, 15, 0, ENE_CONNECT, 2, (u32) &aisle2PlacesEne0FAParam1 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 1, 165, 15, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 185, 15, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 205, 15, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 2, 1 },
    { 1, 225, 15, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 245, 15, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
};
ConnectData aisle3PlacesEne0FAParam0[] = {
    { 30.0f, 116, 1, 1424.0f, 270.0f, 0.0f, 0.0f, 0x0008, 0, 0 },
};
ConnectData aisle3PlacesEne0FAParam1[] = {
    { 30.0f, 117, 1, 77.0f, 81.0f, 0.0f, 0.0f, 0x0008, 0, 0 },
};
EneDef aisle3Places[] = {
    { 1, 78, 15, 0, ENE_CONNECT, 2, (u32) &aisle3PlacesEne0FAParam0 },
    { 1, 338, 15, 0, ENE_CONNECT, 2, (u32) &aisle3PlacesEne0FAParam1 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 1, 165, 15, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 185, 15, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 205, 15, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 2, 1 },
    { 1, 225, 15, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 245, 15, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
};
ConnectData aisle4PlacesEne0FAParam0[] = {
    { 30.0f, 118, 1, 2419.0f, 54.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData aisle4PlacesEne0FAParam1[] = {
    { 30.0f, 139, 1, 23.0f, 69.0f, 0.0f, 0.0f, 0x0008, 0, 0 },
};
EneDef aisle4Places[] = {
    { 1, 78, 15, 0, ENE_CONNECT, 2, (u32) &aisle4PlacesEne0FAParam0 },
    { 1, 338, 15, 0, ENE_CONNECT, 2, (u32) &aisle4PlacesEne0FAParam1 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 1, 165, 15, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 185, 15, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 205, 15, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 2, 1 },
    { 1, 225, 15, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 245, 15, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 265, 16, 0, ENE_ITEM_OOIRI_MAIN, 2, 421 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};

void aisle1Init(void) {
    if (gGameWork.tunePart == 1) {
        aisle1PlacesEne0FAParam0[0].tune = 0;
        aisle1PlacesEne0FAParam1[0].tune = 8;
    } else if (gGameWork.tunePart == 2) {
        aisle1PlacesEne0FAParam0[0].tune = 8;
        aisle1PlacesEne0FAParam1[0].tune = 0;
    }
    stageBuildBasic(0x6D, 1, 145.0f, 15.0f, 0.0f, (u32) aisle1Places, 9);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x96);
    camAiAddLimit(1, 1, 0x104);
    sndPlay(0x520);
    sndSetReverb(1, 0xF);
    gTimeOfDay = 12 << 16;
    camSunRefreshHold(1);
    camSunSetColor(0xFF, 0, 0);
    camSunSetFogColor(0xFF, 0, 0);
    gGameState.fade = 0;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
}

void aisle2Init(void) {
    if (gGameWork.tunePart == 2) {
        aisle2PlacesEne0FAParam0[0].tune = 0;
        aisle2PlacesEne0FAParam1[0].tune = 8;
    } else if (gGameWork.tunePart == 3) {
        aisle2PlacesEne0FAParam0[0].tune = 8;
        aisle2PlacesEne0FAParam1[0].tune = 0;
    }
    stageBuildBasic(0x6D, 1, 145.0f, 15.0f, 0.0f, (u32) aisle2Places, 9);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x96);
    camAiAddLimit(1, 1, 0x104);
    sndPlay(0x520);
    sndSetReverb(1, 0xF);
    gTimeOfDay = 12 << 16;
    camSunRefreshHold(1);
    camSunSetColor(0, 0, 0xFF);
    camSunSetFogColor(0, 0, 0xFF);
    gGameState.fade = 0;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
}

void aisle3Init(void) {
    if (gGameWork.tunePart == 3) {
        aisle3PlacesEne0FAParam0[0].tune = 0;
        aisle3PlacesEne0FAParam1[0].tune = 8;
    } else if (gGameWork.tunePart == 4) {
        aisle3PlacesEne0FAParam0[0].tune = 8;
        aisle3PlacesEne0FAParam1[0].tune = 0;
    }
    stageBuildBasic(0x6D, 1, 145.0f, 15.0f, 0.0f, (u32) aisle3Places, 9);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x96);
    camAiAddLimit(1, 1, 0x104);
    sndPlay(0x520);
    sndSetReverb(1, 0xF);
    gTimeOfDay = 12 << 16;
    camSunRefreshHold(1);
    camSunSetColor(0, 0xFF, 0);
    camSunSetFogColor(0, 0xFF, 0);
    gGameState.fade = 0;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
}

void aisle4Init(void) {
    stageBuildBasic(0x6D, 1, 145.0f, 15.0f, 0.0f, (u32) aisle4Places, 0xD);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x96);
    camAiAddLimit(1, 1, 0x104);
    sndPlay(0x520);
    sndSetReverb(1, 0xF);
    gTimeOfDay = 12 << 16;
    camSunRefreshHold(1);
    camSunSetColor(0xFF, 0, 0xFF);
    camSunSetFogColor(0xFF, 0, 0xFF);
    gGameState.fade = 0;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
}

void aisleClearDemo2Init(void) {
    stageBuildFixedCam(0x70, 0, -690.0f, 0.0f, 830.0f, (u32) aisle4Places, 0xD, -700.0f, 30.0f, 900.0f);
    camAiSetFar(1000);
}

void aisleClearDemo3Init(void) {
    stageBuildFixedCam(0x71, 0, -180.0f, 0.0f, 220.0f, (u32) aisle4Places, 0xD, -210.0f, 30.0f, 310.0f);
    camAiSetFar(1000);
}

void aisleClearDemo4Init(void) {
    stageBuildFixedCam(0x72, 0, 0.0f, 100.0f, 0.0f, (u32) aisle4Places, 0xD, 0.0f, 130.0f, 75.0f);
    camAiSetFar(1000);
}

void aisleClearDemo5Init(void) {
    stageBuildFixedCam(0x73, 0, 33.0f, 0.0f, 650.0f, (u32) aisle4Places, 0xD, -50.0f, 40.0f, 670.0f);
    camAiSetFar(1000);
}
