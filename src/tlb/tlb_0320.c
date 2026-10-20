#include "tlb.h"

extern DoorsData fcRoomCStage11APlacesEne127Param[];
extern ConnectData fcRoomCStage11APlacesEne0FAParam[];
extern ConnectData fcRoomCStage11BPlacesEne0FAParam[];
extern ConnectData fcRoomCStage11CPlacesEne0FAParam[];
extern ConnectData fcRoomCStage11DPlacesEne0FAParam[];
extern u16 fcRoomCStage11APlacesEne1EAParamPath[];
extern PersonData fcRoomCStage11APlacesEne1EAParam[];
extern u16 fcRoomCStage11BPlacesEne1EAParamPath[];
extern PersonData fcRoomCStage11BPlacesEne1EAParam[];
extern PersonData fcRoomCStage11DPlacesEne1EEParam[];
extern u16 fcRoomCStage29PlacesEne1EAParam2Path[];
extern PersonData fcRoomCStage29PlacesEne1EAParam2[];
extern PersonData fcRoomCStage29PlacesEne1EAParam3[];
extern PersonData fcRoomCStage29PlacesEne1EAParam4[];
extern ConnectData fcRoomCStage29PlacesEne0FAParam0[];
extern ConnectData fcRoomCStage29PlacesEne0FAParam1[];
extern DoorsData fcRoomCStage29PlacesEne127Param5[];
extern DoorsData fcRoomCStage29PlacesEne127Param6[];
DoorsData fcRoomCStage11APlacesEne127Param[] = {
    { 0.0f, 15 },
};
ConnectData fcRoomCStage11APlacesEne0FAParam[] = {
    { 15.0f, 138, 0, -858.0f, 5.0f, -73.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData fcRoomCStage11BPlacesEne0FAParam[] = {
    { 15.0f, 138, 0, -587.0f, 5.0f, 50.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData fcRoomCStage11CPlacesEne0FAParam[] = {
    { 15.0f, 138, 0, -446.0f, -11.0f, -2.0f, 0.62831855f, 0x0000, 0, 0 },
};
ConnectData fcRoomCStage11DPlacesEne0FAParam[] = {
    { 15.0f, 138, 0, -336.0f, 0.0f, -31.0f, 0.0f, 0x0000, 0, 0 },
};
u16 fcRoomCStage11APlacesEne1EAParamPath[] = { 0x0004, 0x001E, 0x0002, 0x005A, 0x0001, 0x0000 };
PersonData fcRoomCStage11APlacesEne1EAParam[] = {
    { 729, (u32) &fcRoomCStage11APlacesEne1EAParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef fcRoomCStage11APlaces[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &fcRoomCStage11APlacesEne0FAParam },
    { 0, -22, 12, 0, ENE_DERO_KARASU_YAKUNIN2, 2, (u32) &fcRoomCStage11APlacesEne1EAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &fcRoomCStage11APlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
};
u16 fcRoomCStage11BPlacesEne1EAParamPath[] = { 0x0004, 0x001E, 0x0002, 0x005A, 0x0001, 0x0000 };
PersonData fcRoomCStage11BPlacesEne1EAParam[] = {
    { 730, (u32) &fcRoomCStage11BPlacesEne1EAParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef fcRoomCStage11BPlaces[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &fcRoomCStage11BPlacesEne0FAParam },
    { 0, -22, 12, 0, ENE_DERO_KARASU_YAKUNIN2, 2, (u32) &fcRoomCStage11BPlacesEne1EAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &fcRoomCStage11APlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
};
EneDef fcRoomCStage11CPlaces[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &fcRoomCStage11CPlacesEne0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &fcRoomCStage11APlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
};
PersonData fcRoomCStage11DPlacesEne1EEParam[] = {
    { 0, 0x00000000, { 5, 8 }, 2, 0.5235988f, 1 },
};
EneDef fcRoomCStage11DPlaces[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &fcRoomCStage11DPlacesEne0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &fcRoomCStage11APlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -22, 12, -9, ENE_DERO_KARASU_EVE2, 2, (u32) &fcRoomCStage11DPlacesEne1EEParam },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};
u16 fcRoomCStage29PlacesEne1EAParam2Path[] = { 0x0004, 0x001E, 0x0002, 0x005A, 0x0001, 0x0000, 0x0004, 0x001E, 0x0002, 0x005A, 0x0001, 0x0000, 0x0004, 0x001E, 0x0002, 0x005A, 0x0001, 0x0000 };
PersonData fcRoomCStage29PlacesEne1EAParam2[] = {
    { 732, (u32) &fcRoomCStage29PlacesEne1EAParam2Path, { 5, 8 }, 2, 0.0f, 1 },
};
PersonData fcRoomCStage29PlacesEne1EAParam3[] = {
    { 733, (u32) &fcRoomCStage29PlacesEne1EAParam2Path, { 5, 8 }, 2, 0.0f, 1 },
};
PersonData fcRoomCStage29PlacesEne1EAParam4[] = {
    { 734, (u32) &fcRoomCStage29PlacesEne1EAParam2Path, { 5, 8 }, 2, 0.0f, 1 },
};
ConnectData fcRoomCStage29PlacesEne0FAParam0[] = {
    { 15.0f, 138, 0, 396.0f, -104.0f, 119.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData fcRoomCStage29PlacesEne0FAParam1[] = {
    { 10.0f, 138, 0, 377.0f, -76.0f, 59.0f, -1.5707964f, 0x0000, 0, 0 },
};
DoorsData fcRoomCStage29PlacesEne127Param5[] = {
    { 0.0f, 16 },
};
DoorsData fcRoomCStage29PlacesEne127Param6[] = {
    { 0.0f, 17 },
};
EneDef fcRoomCStage29Places[] = {
    { 0, -24, 5, 145, ENE_CONNECT, 2, (u32) &fcRoomCStage29PlacesEne0FAParam0 },
    { 0, 13, 5, -138, ENE_CONNECT, 2, (u32) &fcRoomCStage29PlacesEne0FAParam1 },
    { 0, 16, 2, 84, ENE_DERO_KARASU_YAKUNIN2, 2, (u32) &fcRoomCStage29PlacesEne1EAParam2 },
    { 0, -63, 15, 14, ENE_DERO_KARASU_YAKUNIN2, 2, (u32) &fcRoomCStage29PlacesEne1EAParam3 },
    { 0, -33, 5, -74, ENE_DERO_KARASU_YAKUNIN2, 2, (u32) &fcRoomCStage29PlacesEne1EAParam4 },
    { 0, -30, 0, 125, ENE_DOORS_SET_47, 2, (u32) &fcRoomCStage29PlacesEne127Param5 },
    { 0, 10, 0, -122, ENE_DOORS_SET_47, 2, (u32) &fcRoomCStage29PlacesEne127Param6 },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, 48, 25, 102, ENE_ROOM_YADOYA_STEAM, 0, 0 },
};

void fcRoomCStage11ABuild(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomCStage11APlaces, 6, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
}

void fcRoomCStage11BBuild(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomCStage11BPlaces, 6, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
}

void fcRoomCStage11CBuild(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomCStage11CPlaces, 5, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
}

void fcRoomCStage11DBuild(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomCStage11DPlaces, 9, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
}

void fcRoomCStage29Build(void) {
    stageBuildFixedCam(0x29, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomCStage29Places, 0xA, 105.0f, 20.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, 0.0f, 0.7853981853f);
}
