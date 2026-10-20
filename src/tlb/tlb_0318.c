#include "tlb.h"

extern DoorsData fcRoomAStage11APlacesEne127Param[];
extern ConnectData fcRoomAStage11APlacesEne0FAParam[];
extern ConnectData fcRoomAStage11BPlacesEne0FAParam[];
extern ConnectData fcRoomAStage11CPlacesEne0FAParam[];
extern ConnectData fcRoomAStage11DPlacesEne0FAParam[];
extern ConnectData fcRoomAStage11EPlacesEne0FAParam[];
extern ConnectData fcRoomAStage11FPlacesEne0FAParam[];
extern PersonData fcRoomAStage11APlacesEne197Param[];
extern u16 fcRoomAStage11BPlacesEne1B9ParamPath[];
extern PersonData fcRoomAStage11BPlacesEne1B9Param[];
extern PersonData fcRoomAStage11CPlacesEne194Param[];
extern PersonData fcRoomAStage11DPlacesEne1B8Param[];
extern u16 fcRoomAStage11EPlacesEne12DParamPath[];
extern PersonData fcRoomAStage11EPlacesEne12DParam[];
extern PersonData fcRoomAStage11FPlacesEne183Param[];
DoorsData fcRoomAStage11APlacesEne127Param[] = {
    { 0.0f, 15 },
};
ConnectData fcRoomAStage11APlacesEne0FAParam[] = {
    { 15.0f, 111, 0, 151.0f, 22.0f, 86.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData fcRoomAStage11BPlacesEne0FAParam[] = {
    { 15.0f, 111, 0, 32.0f, 15.0f, 67.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData fcRoomAStage11CPlacesEne0FAParam[] = {
    { 15.0f, 111, 0, -326.0f, 0.0f, 108.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData fcRoomAStage11DPlacesEne0FAParam[] = {
    { 15.0f, 111, 0, 79.0f, 305.0f, 67.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData fcRoomAStage11EPlacesEne0FAParam[] = {
    { 15.0f, 111, 0, 28.0f, 295.0f, 90.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData fcRoomAStage11FPlacesEne0FAParam[] = {
    { 15.0f, 111, 0, -233.0f, 252.0f, 169.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData fcRoomAStage11APlacesEne197Param[] = {
    { 0, 0x00000000, { 7, 8 }, 2, 1.210824f, 0 },
};
EneDef fcRoomAStage11APlaces[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &fcRoomAStage11APlacesEne0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &fcRoomAStage11APlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 22, 24, -43, ENE_BLACK, 2, 1 },
    { 0, 0, 0, 0, ENE_DEMO_KAKURENBO, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, -25, 10, 6, ENE_KENSUKE, 2, (u32) &fcRoomAStage11APlacesEne197Param },
    { 0, 0, 0, 0, ENE_KENSUKE_MONEY, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};
u16 fcRoomAStage11BPlacesEne1B9ParamPath[] = { 0x0004, 0x003C, 0x0002, 0x005A, 0x0001, 0x0000 };
PersonData fcRoomAStage11BPlacesEne1B9Param[] = {
    { 662, (u32) &fcRoomAStage11BPlacesEne1B9ParamPath, { 5, 8 }, 2, 1.484896f, 0 },
};
EneDef fcRoomAStage11BPlaces[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &fcRoomAStage11BPlacesEne0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &fcRoomAStage11APlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -36, 9, 25, ENE_SHOP_KYAKU_OBASAN, 2, (u32) &fcRoomAStage11BPlacesEne1B9Param },
    { 0, -54, 22, 11, ENE_GREEN, 2, 1 },
    { 0, 0, 0, 0, ENE_DEMO_KAKURENBO, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
};
PersonData fcRoomAStage11CPlacesEne194Param[] = {
    { 0, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef fcRoomAStage11CPlaces[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &fcRoomAStage11CPlacesEne0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &fcRoomAStage11APlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 22, 24, -43, ENE_BLACK, 2, 3 },
    { 0, 0, 0, 0, ENE_DEMO_KAKURENBO, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, 20, 0, 20, ENE_KARASU_LADY, 2, (u32) &fcRoomAStage11CPlacesEne194Param },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};
PersonData fcRoomAStage11DPlacesEne1B8Param[] = {
    { 660, 0x00000000, { 5, 8 }, 2, 0.816071f, 0 },
};
EneDef fcRoomAStage11DPlaces[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &fcRoomAStage11DPlacesEne0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &fcRoomAStage11APlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -5, 6, 6, ENE_SHOP_KYAKU_DEBU, 2, (u32) &fcRoomAStage11DPlacesEne1B8Param },
    { 0, 22, 24, -43, ENE_BLACK, 2, 2 },
    { 0, 0, 0, 0, ENE_DEMO_KAKURENBO, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
};
u16 fcRoomAStage11EPlacesEne12DParamPath[] = { 0x0004, 0x003C, 0x0002, 0x00B4, 0x0001, 0x0000 };
PersonData fcRoomAStage11EPlacesEne12DParam[] = {
    { 661, (u32) &fcRoomAStage11EPlacesEne12DParamPath, { 5, 8 }, 2, 0.595184f, 0 },
};
EneDef fcRoomAStage11EPlaces[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &fcRoomAStage11EPlacesEne0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &fcRoomAStage11APlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 13, 7, -6, ENE_SHOP_KYAKU_OYAJI, 2, (u32) &fcRoomAStage11EPlacesEne12DParam },
    { 0, -54, 22, 11, ENE_GREEN, 2, 2 },
    { 0, 0, 0, 0, ENE_DEMO_KAKURENBO, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
};
PersonData fcRoomAStage11FPlacesEne183Param[] = {
    { 0, 0x00000000, { 5, 8 }, 2, 0.5235988f, 0 },
};
EneDef fcRoomAStage11FPlaces[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &fcRoomAStage11FPlacesEne0FAParam },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &fcRoomAStage11APlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -55, 27, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -54, 22, 11, ENE_GREEN, 2, 3 },
    { 0, 0, 0, 0, ENE_DEMO_KAKURENBO, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, -18, 12, 10, ENE_KODOMO_RED, 2, (u32) &fcRoomAStage11FPlacesEne183Param },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};

void fcRoomAStage11ABuild(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomAStage11APlaces, 0xD, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        eneMakeEx(NULL, ENE_DEMO_KAKURENBO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
    }
}

void fcRoomAStage11BBuild(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomAStage11BPlaces, 9, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        eneMakeEx(NULL, ENE_DEMO_KAKURENBO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
    }
}

void fcRoomAStage11CBuild(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomAStage11CPlaces, 0xC, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        eneMakeEx(NULL, ENE_DEMO_KAKURENBO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
    }
}

void fcRoomAStage11DBuild(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomAStage11DPlaces, 9, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        eneMakeEx(NULL, ENE_DEMO_KAKURENBO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
    }
}

void fcRoomAStage11EBuild(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomAStage11EPlaces, 9, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        eneMakeEx(NULL, ENE_DEMO_KAKURENBO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
    }
}

void fcRoomAStage11FBuild(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) fcRoomAStage11FPlaces, 0xC, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        eneMakeEx(NULL, ENE_DEMO_KAKURENBO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
    }
}
