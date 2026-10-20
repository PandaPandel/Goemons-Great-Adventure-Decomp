#include "tlb.h"

/* Archive file 316: the stage 5 town's houses (gStageList "5 MINKA1" .. "5 MINKA5", "5 BANNYA"). */

extern DoorsData stage5MinkaPlaces1Ene127Param[];
extern ConnectData stage5MinkaPlaces1Ene0FAParam[];
extern u16 stage5MinkaPlaces1Ene1BBParamPath[];
extern PersonData stage5MinkaPlaces1Ene1BBParam[];
extern ConnectData stage5MinkaPlaces2Ene0FAParam[];
extern u16 stage5MinkaPlaces2Ene1CDParamPath[];
extern PersonData stage5MinkaPlaces2Ene1CDParam[];
extern ConnectData stage5MinkaPlaces3Ene0FAParam[];
extern PersonData stage5MinkaPlaces3Ene1B9Param[];
extern ConnectData stage5MinkaPlaces4Ene0FAParam[];
extern DoorsData stage5MinkaPlaces4Ene127Param[];
extern PersonData stage5MinkaPlaces4Ene1C7Param[];
extern ConnectData stage5MinkaPlaces5Ene0FAParam[];
extern DoorsData stage5MinkaPlaces5Ene127Param[];
extern PersonData stage5MinkaPlaces5Ene1CAParam[];
extern ConnectData stage5MinkaBannyaPlacesEne0FAParam[];
extern DoorsData stage5MinkaBannyaPlacesEne127Param[];
extern PersonData stage5MinkaBannyaPlacesEne1B9Param[];
DoorsData stage5MinkaPlaces1Ene127Param[] = {
    { 0.0f, 19 },
};
ConnectData stage5MinkaPlaces1Ene0FAParam[] = {
    { 10.0f, 110, 0, -281.0f, 0.0f, 978.0f, 0.0f, 0x0000, 0, 0 },
};
u16 stage5MinkaPlaces1Ene1BBParamPath[] = { 0x0002, 0x0000, 0x0004, 0x003C, 0x0002, 0x0028, 0x0004, 0x003C, 0x0001, 0x0000 };
PersonData stage5MinkaPlaces1Ene1BBParam[] = {
    { 704, (u32) &stage5MinkaPlaces1Ene1BBParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef stage5MinkaPlaces1[] = {
    { 0, -48, 10, 92, ENE_CONNECT, 2, (u32) &stage5MinkaPlaces1Ene0FAParam },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage5MinkaPlaces1Ene127Param },
    { 0, -39, 0, 29, ENE_SHOP_KYAKU_OTOKO, 2, (u32) &stage5MinkaPlaces1Ene1BBParam },
};
ConnectData stage5MinkaPlaces2Ene0FAParam[] = {
    { 10.0f, 110, 0, -221.0f, 0.0f, 991.0f, 0.0f, 0x0000, 0, 0 },
};
u16 stage5MinkaPlaces2Ene1CDParamPath[] = { 0x0000, 0xFFEB, 0x0008, 0x0006, 0x0000, 0x0021, 0x000A, 0x0006, 0x0000, 0x001B, 0xFFF1, 0x0006, 0x0000, 0xFFD9, 0xFFD9, 0x0006, 0x0001, 0x0000 };
PersonData stage5MinkaPlaces2Ene1CDParam[] = {
    { 0, (u32) &stage5MinkaPlaces2Ene1CDParamPath, { 8, 8 }, 2, 0.0f, 0 },
};
EneDef stage5MinkaPlaces2[] = {
    { 0, -48, 10, 92, ENE_CONNECT, 2, (u32) &stage5MinkaPlaces2Ene0FAParam },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage5MinkaPlaces1Ene127Param },
    { 0, -39, 10, -39, ENE_OBISU_MARU2, 2, (u32) &stage5MinkaPlaces2Ene1CDParam },
    { 0, 0, 0, 0, ENE_OBISU_EFFECT, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};
ConnectData stage5MinkaPlaces3Ene0FAParam[] = {
    { 10.0f, 110, 0, -159.0f, 0.0f, 1002.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stage5MinkaPlaces3Ene1B9Param[] = {
    { 705, 0x00000000, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef stage5MinkaPlaces3[] = {
    { 0, -48, 10, 92, ENE_CONNECT, 2, (u32) &stage5MinkaPlaces3Ene0FAParam },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, -47, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage5MinkaPlaces1Ene127Param },
    { 0, 4, 10, -13, ENE_SHOP_KYAKU_OBASAN, 2, (u32) &stage5MinkaPlaces3Ene1B9Param },
};
ConnectData stage5MinkaPlaces4Ene0FAParam[] = {
    { 15.0f, 110, 0, 186.0f, 0.0f, 1000.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stage5MinkaPlaces4Ene127Param[] = {
    { 0.0f, 15 },
};
PersonData stage5MinkaPlaces4Ene1C7Param[] = {
    { 0, 0x00000000, { 8, 8 }, 0, 0.0f, 0 },
};
EneDef stage5MinkaPlaces4[] = {
    { 0, 14, 0, 98, ENE_CONNECT, 2, (u32) &stage5MinkaPlaces4Ene0FAParam },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 6, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage5MinkaPlaces4Ene127Param },
    { 0, -14, 10, -7, ENE_DTSK_OKAMI, 2, (u32) &stage5MinkaPlaces4Ene1C7Param },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};
ConnectData stage5MinkaPlaces5Ene0FAParam[] = {
    { 15.0f, 110, 0, 715.0f, 2.0f, 799.0f, 0.9424778f, 0x0000, 0, 0 },
};
DoorsData stage5MinkaPlaces5Ene127Param[] = {
    { 0.0f, 18 },
};
PersonData stage5MinkaPlaces5Ene1CAParam[] = {
    { 0, 0x00000000, { 10, 8 }, 0, 0.0f, 0 },
};
EneDef stage5MinkaPlaces5[] = {
    { 0, 16, 8, 137, ENE_CONNECT, 2, (u32) &stage5MinkaPlaces5Ene0FAParam },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 15, 0, 121, ENE_DOORS_SET_47, 2, (u32) &stage5MinkaPlaces5Ene127Param },
    { 0, 44, 2, 22, ENE_DTSK_MI_ZUKIN, 2, (u32) &stage5MinkaPlaces5Ene1CAParam },
};
ConnectData stage5MinkaBannyaPlacesEne0FAParam[] = {
    { 15.0f, 110, 0, -160.0f, 0.0f, 658.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stage5MinkaBannyaPlacesEne127Param[] = {
    { 0.0f, 20 },
};
PersonData stage5MinkaBannyaPlacesEne1B9Param[] = {
    { 708, 0x00000000, { 5, 8 }, 1, 0.9424778f, 1 },
};
EneDef stage5MinkaBannyaPlaces[] = {
    { 0, 15, 0, 90, ENE_CONNECT, 2, (u32) &stage5MinkaBannyaPlacesEne0FAParam },
    { 0, 5, 0, 73, ENE_DOORS_SET_47, 2, (u32) &stage5MinkaBannyaPlacesEne127Param },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -36, 27, 10, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -11, 10, 10, ENE_SHOP_KYAKU_OBASAN, 2, (u32) &stage5MinkaBannyaPlacesEne1B9Param },
};

/* "5 MINKA1" */
void stage5MinkaBuild1(void) {
    stageBuildFixedCam(0x2C, 0, 0.0f, 0.0f, 0.0f, (u32) stage5MinkaPlaces1, 4, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}

/* "5 MINKA2" */
void stage5MinkaBuild2(void) {
    stageBuildFixedCam(0x2C, 0, 0.0f, 0.0f, 0.0f, (u32) stage5MinkaPlaces2, 8, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}

/* "5 MINKA3" */
void stage5MinkaBuild3(void) {
    stageBuildFixedCam(0x2C, 0, 0.0f, 0.0f, 0.0f, (u32) stage5MinkaPlaces3, 4, 85.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(20.0f, 20.0f, 0.6597344279f);
}

/* "5 MINKA4" */
void stage5MinkaBuild4(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 0.0f, 0.0f, (u32) stage5MinkaPlaces4, 7, 85.0f, 20.0f, 10.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 12.0f, 0.6597344279f);
}

/* "5 MINKA5" */
void stage5MinkaBuild5(void) {
    gGameWork.geoCacheSize = 40;
    stageBuildFixedCam(0x2A, 0, 0.0f, 0.0f, 0.0f, (u32) stage5MinkaPlaces5, 4, 145.0f, 20.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, -5.0f, 0.7853981853f);
}

/* "5 BANNYA" */
void stage5MinkaBuildBannya(void) {
    stageBuildFixedCam(0x42, 0, 0.0f, 0.0f, 0.0f, (u32) stage5MinkaBannyaPlaces, 6, 85.0f, 20.0f, 50.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 37.0f, 0.5026548505f);
}
