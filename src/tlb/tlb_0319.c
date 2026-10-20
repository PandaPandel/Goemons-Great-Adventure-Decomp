#include "tlb.h"

extern ConnectData fcRoomBStage0FPlacesEne0FAParam0[];
extern ConnectData fcRoomBStage0FPlacesEne0FAParam1[];
extern PersonData fcRoomBStage0FPlacesEne104Param[];
extern u16 fcRoomBStage0FPlacesEne1EAParamPath[];
extern PersonData fcRoomBStage0FPlacesEne1EAParam[];
extern ConnectData fcRoomBStage0DPlacesEne0FAParam[];
extern DoorsData fcRoomBStage0DPlacesEne127Param[];
extern u16 fcRoomBStage0DPlacesEne19FParamPath[];
extern PersonData fcRoomBStage0DPlacesEne19FParam[];
extern ConnectData fcRoomBStage0EPlacesEne0FAParam[];
extern PersonData fcRoomBStage0EPlacesEne19CParam[];
extern u16 fcRoomBStage0EPlacesEne1EAParam8Path[];
extern PersonData fcRoomBStage0EPlacesEne1EAParam8[];
extern u16 fcRoomBStage0EPlacesEne1EAParam10Path[];
extern PersonData fcRoomBStage0EPlacesEne1EAParam10[];
extern ConnectData fcRoomBStage43PlacesEne0FAParam[];
extern u16 fcRoomBStage43PlacesEne131ParamPath[];
extern PersonData fcRoomBStage43PlacesEne131Param[];
extern ConnectData fcRoomBStage2EPlacesEne0FAParam0[];
extern ConnectData fcRoomBStage2EPlacesEne0FAParam1[];
extern ConnectData fcRoomBStage2EPlacesEne0FAParam2[];
extern PersonData fcRoomBStage2EPlacesEne128Param[];
extern ConnectData fcRoomBStage10PlacesEne0FAParam[];
extern PersonData fcRoomBStage10PlacesEne19DParam[];
extern DoorsData fcRoomBStage10PlacesEne127Param[];
ConnectData fcRoomBStage0FPlacesEne0FAParam0[] = {
    { 15.0f, 138, 0, -794.0f, -6.0f, -44.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData fcRoomBStage0FPlacesEne0FAParam1[] = {
    { 15.0f, 138, 0, -794.0f, -6.0f, -44.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData fcRoomBStage0FPlacesEne104Param[] = {
    { 787, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
u16 fcRoomBStage0FPlacesEne1EAParamPath[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData fcRoomBStage0FPlacesEne1EAParam[] = {
    { 728, (u32) &fcRoomBStage0FPlacesEne1EAParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef fcRoomBStage0FPlaces[] = {
    { 0, 45, 0, 105, ENE_CONNECT, 2, (u32) &fcRoomBStage0FPlacesEne0FAParam0 },
    { 0, -15, 0, 105, ENE_CONNECT, 2, (u32) &fcRoomBStage0FPlacesEne0FAParam1 },
    { 0, 0, 0, 0, ENE_SHOP_TENGU, 0, 0 },
    { 0, 0, -1, 30, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_YOROZU_DEBU, 0, 0 },
    { 0, 19, 10, -15, ENE_TABLE_YOROZUYA, 2, (u32) &fcRoomBStage0FPlacesEne104Param },
    { 0, 44, 13, -13, ENE_SHOP_YOROZU_ITEM4, 2, 0 },
    { 0, 59, 13, -13, ENE_SHOP_YOROZU_ITEM3, 2, 0 },
    { 0, 74, 13, -13, ENE_SHOP_YOROZU_KIN_YOROI, 2, 0 },
    { 0, 0, 1, 0, ENE_DERO_KARASU_YAKUNIN2, 2, (u32) &fcRoomBStage0FPlacesEne1EAParam },
};
ConnectData fcRoomBStage0DPlacesEne0FAParam[] = {
    { 10.0f, 138, 0, -254.0f, -91.0f, 133.0f, 0.0f, 0x0000, 0, 8 },
};
DoorsData fcRoomBStage0DPlacesEne127Param[] = {
    { 0.0f, 10 },
};
u16 fcRoomBStage0DPlacesEne19FParamPath[] = { 0x0002, 0x010E, 0x0001, 0x0000 };
PersonData fcRoomBStage0DPlacesEne19FParam[] = {
    { 790, (u32) &fcRoomBStage0DPlacesEne19FParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef fcRoomBStage0DPlaces[] = {
    { 0, 32, 0, 100, ENE_CONNECT, 2, (u32) &fcRoomBStage0DPlacesEne0FAParam },
    { 0, 31, 0, 79, ENE_DOORS_SET_47, 2, (u32) &fcRoomBStage0DPlacesEne127Param },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -50, 18, -12, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 40, 0, 2, ENE_SHOP_CHA_BABA, 2, (u32) &fcRoomBStage0DPlacesEne19FParam },
    { 0, 0, 1, 0, ENE_CHAMISE, 0, 0 },
};
ConnectData fcRoomBStage0EPlacesEne0FAParam[] = {
    { 15.0f, 138, 0, -248.0f, 12.0f, -42.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData fcRoomBStage0EPlacesEne19CParam[] = {
    { 788, 0x00000000, { 4, 8 }, 0, 0.0f, 0 },
};
u16 fcRoomBStage0EPlacesEne1EAParam8Path[] = { 0x0002, 0x0000, 0x0001, 0x0000 };
PersonData fcRoomBStage0EPlacesEne1EAParam8[] = {
    { 789, (u32) &fcRoomBStage0EPlacesEne1EAParam8Path, { 5, 8 }, 2, 1.0f, 0 },
};
u16 fcRoomBStage0EPlacesEne1EAParam10Path[] = { 0x0004, 0x001E, 0x0002, 0x005A, 0x0001, 0x0000 };
PersonData fcRoomBStage0EPlacesEne1EAParam10[] = {
    { 731, (u32) &fcRoomBStage0EPlacesEne1EAParam10Path, { 5, 8 }, 2, 1.0f, 0 },
};
EneDef fcRoomBStage0EPlaces[] = {
    { 0, 40, 0, 105, ENE_CONNECT, 2, (u32) &fcRoomBStage0EPlacesEne0FAParam },
    { 0, 0, -1, 15, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH44A, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH44B, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DERODERO, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, 15, 17, 70, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -11, 0, 58, ENE_TABLE_GANSO, 2, (u32) &fcRoomBStage0EPlacesEne19CParam },
    { 0, -40, 5, 57, ENE_DERO_KARASU_YAKUNIN2, 2, (u32) &fcRoomBStage0EPlacesEne1EAParam8 },
    { 0, -11, 0, 58, ENE_SHOP_TENGU_TURN, 0, 0 },
    { 0, -37, 0, -4, ENE_DERO_KARASU_YAKUNIN2, 2, (u32) &fcRoomBStage0EPlacesEne1EAParam10 },
};
ConnectData fcRoomBStage43PlacesEne0FAParam[] = {
    { 15.0f, 138, 0, 325.0f, 0.0f, -43.0f, 0.0f, 0x0000, 0, 0 },
};
u16 fcRoomBStage43PlacesEne131ParamPath[] = { 0x0002, 0x0014, 0x0004, 0x001E, 0x0002, 0x003C, 0x0004, 0x001E, 0x0001, 0x0000 };
PersonData fcRoomBStage43PlacesEne131Param[] = {
    { 45, (u32) &fcRoomBStage43PlacesEne131ParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef fcRoomBStage43Places[] = {
    { 0, -45, 0, 90, ENE_CONNECT, 2, (u32) &fcRoomBStage43PlacesEne0FAParam },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, 43, 27, -8, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 0, 10, 0, ENE_SHOP_SARAYA_MANIA, 2, (u32) &fcRoomBStage43PlacesEne131Param },
};
ConnectData fcRoomBStage2EPlacesEne0FAParam0[] = {
    { 10.0f, 138, 0, 484.0f, 17.0f, -61.0f, 0.0f, 0x0007, 0, 0 },
};
ConnectData fcRoomBStage2EPlacesEne0FAParam1[] = {
    { 10.0f, 138, 0, 484.0f, 17.0f, -61.0f, 0.0f, 0x0007, 0, 0 },
};
ConnectData fcRoomBStage2EPlacesEne0FAParam2[] = {
    { 10.0f, 138, 0, 484.0f, 17.0f, -61.0f, 0.0f, 0x0007, 0, 0 },
};
PersonData fcRoomBStage2EPlacesEne128Param[] = {
    { 44, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef fcRoomBStage2EPlaces[] = {
    { 0, -40, 10, 135, ENE_CONNECT, 2, (u32) &fcRoomBStage2EPlacesEne0FAParam0 },
    { 0, 0, 10, 135, ENE_CONNECT, 2, (u32) &fcRoomBStage2EPlacesEne0FAParam1 },
    { 0, 40, 10, 135, ENE_CONNECT, 2, (u32) &fcRoomBStage2EPlacesEne0FAParam2 },
    { 0, 0, -1, -80, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_PURA, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_DANCER, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_DANCER, 0, 0 },
    { 0, 0, 0, 0, ENE_URANAI_DECO, 0, 0 },
    { 0, 0, 0, 0, ENE_URANAI_CAMERA, 0, 0 },
    { 0, 40, 5, 0, ENE_URANAI_TABLE, 2, (u32) &fcRoomBStage2EPlacesEne128Param },
};
ConnectData fcRoomBStage10PlacesEne0FAParam[] = {
    { 10.0f, 138, 0, 222.0f, 5.0f, -173.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData fcRoomBStage10PlacesEne19DParam[] = {
    { 751, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
DoorsData fcRoomBStage10PlacesEne127Param[] = {
    { 0.0f, 13 },
};
EneDef fcRoomBStage10Places[] = {
    { 0, 60, 0, 70, ENE_CONNECT, 2, (u32) &fcRoomBStage10PlacesEne0FAParam },
    { 0, 0, 0, 0, ENE_SHOP_TENGU, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -29, 27, 6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 60, 0, 48, ENE_DOORS_SET_47, 2, (u32) &fcRoomBStage10PlacesEne127Param },
    { 0, 0, 0, 0, ENE_SHOP_YADOYA, 0, 0 },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 57, 4, -32, ENE_TABLE_YADOYA, 2, (u32) &fcRoomBStage10PlacesEne19DParam },
};

void fcRoomBStage0FBuild(void) {
    stageBuildFixedCam(0xF, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomBStage0FPlaces, 0xA, 120.0f, 20.0f, 50.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, 36.0f, 0.471238941f);
}

void fcRoomBStage0DBuild(void) {
    stageBuildFixedCam(0xD, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomBStage0DPlaces, 7, 0.0f, 20.0f, -75.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, -40.0f, 0.6283185482f);
    camSunSetTunes4(0x30, 0x2D, 0x2E, 0x2F);
    camSunSetDawnSound(0x42F);
}

void fcRoomBStage0EBuild(void) {
    stageBuildFixedCam(0xE, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomBStage0EPlaces, 0xB, 0.0f, 20.0f, -75.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 40.0f, 0.6283185482f);
}

void fcRoomBStage43Build(void) {
    stageBuildFixedCam(0x43, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomBStage43Places, 5, 85.0f, 20.0f, 30.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 20.0f, 0.5026548505f);
}

void fcRoomBStage2EBuild(void) {
    stageBuildFixedCam(0x2E, 0, 0.0f, 0.0f, 77.0f, (u32) fcRoomBStage2EPlaces, 0xA, -180.0f, 20.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(10.0f, 0.0f, 0.471238941f);
}

void fcRoomBStage10Build(void) {
    stageBuildFixedCam(0x10, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomBStage10Places, 8, 120.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, 5.0f, 0.5026548505f);
}
