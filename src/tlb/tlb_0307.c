#include "tlb.h"

extern ConnectData stg0FPlaces307Ene0FAParam1[];
extern ConnectData stg0FPlaces307Ene0FAParam0[];
extern PersonData stg0FPlaces307Ene104Param[];
extern u16 stg0FPlaces307Ene1BBParamPath[];
extern PersonData stg0FPlaces307Ene1BBParam[];
extern u16 stg0FPlaces307Ene1B9ParamPath[];
extern PersonData stg0FPlaces307Ene1B9Param[];
extern ConnectData stg0DPlaces307Ene0FAParam[];
extern DoorsData stg0DPlaces307Ene127Param[];
extern u16 stg0DPlaces307Ene19FParamPath[];
extern PersonData stg0DPlaces307Ene19FParam[];
extern ConnectData stg10Places307Ene0FAParam[];
extern PersonData stg10Places307Ene19DParam[];
extern DoorsData stg10Places307Ene127Param[];
extern u16 stg10Places307Ene1BAParamPath[];
extern PersonData stg10Places307Ene1BAParam[];
extern ConnectData stg0EPlaces307Ene0FAParam[];
extern PersonData stg0EPlaces307Ene19CParam[];
extern PersonData stg0EPlaces307Ene1A1Param[];
extern PersonData stg0EPlaces307Ene1B8Param[];
extern PersonData stg0EPlaces307Ene12DParam[];
extern ConnectData stg2APlaces307Ene0FAParam[];
extern DoorsData stg2APlaces307Ene127Param[];
extern u16 stg2APlaces307Ene13EParamPath[];
extern PersonData stg2APlaces307Ene13EParam[];
extern PersonData stg2APlaces307Ene1B9Param[];
extern PersonData stg2APlaces307Ene196Param[];
extern ConnectData stg2EPlaces307Ene0FAParam0[];
extern ConnectData stg2EPlaces307Ene0FAParam1[];
extern ConnectData stg2EPlaces307Ene0FAParam2[];
extern PersonData stg2EPlaces307Ene128Param[];
ConnectData stg0FPlaces307Ene0FAParam1[] = {
    { 15.0f, 71, 0, -570.0f, 0.0f, -64.0f, 0.31415927f, 0x0000, 0, 0 },
};
ConnectData stg0FPlaces307Ene0FAParam0[] = {
    { 15.0f, 71, 0, -518.0f, 0.0f, -92.0f, 0.31415927f, 0x0000, 0, 0 },
};
PersonData stg0FPlaces307Ene104Param[] = {
    { 766, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
u16 stg0FPlaces307Ene1BBParamPath[] = { 0x0000, 0xFFCE, 0x0016, 0x0003, 0x0004, 0x001E, 0x0002, 0x005A, 0x0000, 0x003C, 0x0016, 0x0003, 0x0002, 0x00B4, 0x0004, 0x0096, 0x0002, 0x010E, 0x0001, 0x0000 };
PersonData stg0FPlaces307Ene1BBParam[] = {
    { 579, (u32) &stg0FPlaces307Ene1BBParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
u16 stg0FPlaces307Ene1B9ParamPath[] = { 0x0004, 0x001E, 0x0001, 0x0000 };
PersonData stg0FPlaces307Ene1B9Param[] = {
    { 580, (u32) &stg0FPlaces307Ene1B9ParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stg0FPlaces307[] = {
    { 0, 45, 0, 105, ENE_CONNECT, 2, (u32) &stg0FPlaces307Ene0FAParam0 },
    { 0, -15, 0, 105, ENE_CONNECT, 2, (u32) &stg0FPlaces307Ene0FAParam1 },
    { 0, 0, -1, 30, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_YOROZU_DEBU, 0, 0 },
    { 0, 19, 10, -15, ENE_TABLE_YOROZUYA, 2, (u32) &stg0FPlaces307Ene104Param },
    { 0, 44, 13, -13, ENE_SHOP_YOROZU_ITEM1, 2, 0 },
    { 0, 59, 13, -13, ENE_SHOP_YOROZU_ITEM3, 2, 0 },
    { 0, 74, 13, -13, ENE_SHOP_YOROZU_GIN_YOROI, 2, 0 },
    { 0, 0, 1, 0, ENE_SHOP_KYAKU_OTOKO, 2, (u32) &stg0FPlaces307Ene1BBParam },
    { 0, -41, 10, -15, ENE_SHOP_KYAKU_OBASAN, 2, (u32) &stg0FPlaces307Ene1B9Param },
    { 0, -21, 10, -13, ENE_ITEM_OOIRI_MAIN, 2, 428 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};
ConnectData stg0DPlaces307Ene0FAParam[] = {
    { 10.0f, 71, 0, -390.0f, 0.0f, 86.0f, 0.0f, 0x0000, 0, 8 },
};
DoorsData stg0DPlaces307Ene127Param[] = {
    { 0.0f, 10 },
};
u16 stg0DPlaces307Ene19FParamPath[] = { 0x0002, 0x010E, 0x0001, 0x0000 };
PersonData stg0DPlaces307Ene19FParam[] = {
    { 774, (u32) &stg0DPlaces307Ene19FParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stg0DPlaces307[] = {
    { 0, 32, 0, 100, ENE_CONNECT, 2, (u32) &stg0DPlaces307Ene0FAParam },
    { 0, 31, 0, 79, ENE_DOORS_SET_47, 2, (u32) &stg0DPlaces307Ene127Param },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -50, 18, -12, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 40, 0, 2, ENE_SHOP_CHA_BABA, 2, (u32) &stg0DPlaces307Ene19FParam },
    { 0, 0, 1, 0, ENE_CHAMISE, 0, 0 },
};
ConnectData stg10Places307Ene0FAParam[] = {
    { 10.0f, 71, 0, -212.0f, 0.0f, 165.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stg10Places307Ene19DParam[] = {
    { 750, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
DoorsData stg10Places307Ene127Param[] = {
    { 0.0f, 13 },
};
u16 stg10Places307Ene1BAParamPath[] = { 0x0000, 0x000D, 0x0020, 0x0001, 0x0004, 0x003C, 0x0002, 0x00B4, 0x0000, 0x000D, 0xFFF6, 0x0001, 0x0002, 0x005A, 0x0004, 0x003C, 0x0002, 0x0000, 0x0001, 0x0000 };
PersonData stg10Places307Ene1BAParam[] = {
    { 581, (u32) &stg10Places307Ene1BAParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stg10Places307[] = {
    { 0, 60, 0, 70, ENE_CONNECT, 2, (u32) &stg10Places307Ene0FAParam },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -29, 27, 6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 60, 0, 48, ENE_DOORS_SET_47, 2, (u32) &stg10Places307Ene127Param },
    { 0, 0, 0, 0, ENE_SHOP_YADOYA, 0, 0 },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 57, 4, -32, ENE_TABLE_YADOYA, 2, (u32) &stg10Places307Ene19DParam },
    { 0, 0, 1, 0, ENE_SHOP_KYAKU_BOUZU, 2, (u32) &stg10Places307Ene1BAParam },
};
ConnectData stg0EPlaces307Ene0FAParam[] = {
    { 15.0f, 71, 0, -86.0f, 0.0f, 158.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stg0EPlaces307Ene19CParam[] = {
    { 768, 0x00000000, { 4, 8 }, 0, 0.0f, 0 },
};
PersonData stg0EPlaces307Ene1A1Param[] = {
    { 769, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
PersonData stg0EPlaces307Ene1B8Param[] = {
    { 583, 0x00000000, { 5, 8 }, 2, 3.1415927f, 0 },
};
PersonData stg0EPlaces307Ene12DParam[] = {
    { 582, 0x00000000, { 5, 8 }, 2, 3.1415927f, 0 },
};
EneDef stg0EPlaces307[] = {
    { 0, 40, 0, 105, ENE_CONNECT, 2, (u32) &stg0EPlaces307Ene0FAParam },
    { 0, 0, -1, 15, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH42A, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH42B, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH42C, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, 15, 17, 70, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -11, 0, 58, ENE_TABLE_GANSO, 2, (u32) &stg0EPlaces307Ene19CParam },
    { 0, -40, 5, 57, ENE_SHOP_GANSO_ITA, 2, (u32) &stg0EPlaces307Ene1A1Param },
    { 0, -11, 0, 58, ENE_SHOP_GANSO_GIRL, 0, 0 },
    { 0, 26, 3, 33, ENE_SHOP_KYAKU_DEBU, 2, (u32) &stg0EPlaces307Ene1B8Param },
    { 0, -27, 5, -11, ENE_SHOP_KYAKU_OYAJI, 2, (u32) &stg0EPlaces307Ene12DParam },
};
ConnectData stg2APlaces307Ene0FAParam[] = {
    { 15.0f, 71, 0, 546.0f, -29.0f, 185.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData stg2APlaces307Ene127Param[] = {
    { 0.0f, 18 },
};
u16 stg2APlaces307Ene13EParamPath[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData stg2APlaces307Ene13EParam[] = {
    { 588, (u32) &stg2APlaces307Ene13EParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
PersonData stg2APlaces307Ene1B9Param[] = {
    { 587, 0x00000000, { 5, 8 }, 2, 1.239451f, 0 },
};
PersonData stg2APlaces307Ene196Param[] = {
    { 589, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stg2APlaces307[] = {
    { 0, 16, 8, 137, ENE_CONNECT, 2, (u32) &stg2APlaces307Ene0FAParam },
    { 0, 15, 0, 121, ENE_DOORS_SET_47, 2, (u32) &stg2APlaces307Ene127Param },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, -26, 28, -110, ENE_ITEM_OOIRI_MAIN, 2, 408 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, -18, 2, 82, ENE_KODOMO, 2, (u32) &stg2APlaces307Ene13EParam },
    { 0, -9, 10, 41, ENE_SHOP_KYAKU_OBASAN, 2, (u32) &stg2APlaces307Ene1B9Param },
    { 0, -16, 10, 115, ENE_TOKAGE, 2, (u32) &stg2APlaces307Ene196Param },
};
ConnectData stg2EPlaces307Ene0FAParam0[] = {
    { 10.0f, 71, 0, -350.0f, 140.0f, -110.0f, 0.0f, 0x0007, 0, 0 },
};
ConnectData stg2EPlaces307Ene0FAParam1[] = {
    { 10.0f, 71, 0, -296.0f, 140.0f, -110.0f, 0.0f, 0x0007, 0, 0 },
};
ConnectData stg2EPlaces307Ene0FAParam2[] = {
    { 10.0f, 71, 0, -242.0f, 140.0f, -110.0f, 0.0f, 0x0007, 0, 0 },
};
PersonData stg2EPlaces307Ene128Param[] = {
    { 41, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stg2EPlaces307[] = {
    { 0, -40, 10, 135, ENE_CONNECT, 2, (u32) &stg2EPlaces307Ene0FAParam0 },
    { 0, 0, 10, 135, ENE_CONNECT, 2, (u32) &stg2EPlaces307Ene0FAParam1 },
    { 0, 40, 10, 135, ENE_CONNECT, 2, (u32) &stg2EPlaces307Ene0FAParam2 },
    { 0, 0, -1, -80, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_URANAI_DECO, 0, 0 },
    { 0, 0, 0, 0, ENE_URANAI_CAMERA, 0, 0 },
    { 0, 40, 5, 0, ENE_URANAI_TABLE, 2, (u32) &stg2EPlaces307Ene128Param },
    { 0, 0, 0, 0, ENE_SHOP_PURA, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_DANCER, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_DANCER, 0, 0 },
};

void stg0FBuild307(void) {
    stageBuildFixedCam(0xF, 0, 0.0f, 0.0f, 0.0f, (u32) stg0FPlaces307, 0xE, 120.0f, 20.0f, 50.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, 36.0f, 0.471238941f);
}

void stg0DBuild307(void) {
    stageBuildFixedCam(0xD, 0, 0.0f, 0.0f, 0.0f, (u32) stg0DPlaces307, 7, 0.0f, 20.0f, -75.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, -40.0f, 0.6283185482f);
    camSunSetTunes4(0x84, 0x81, 0x82, 0x83);
    camSunSetDawnSound(0x42F);
}

void stg10Build307(void) {
    stageBuildFixedCam(0x10, 0, 0.0f, 0.0f, 0.0f, (u32) stg10Places307, 8, 120.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, 5.0f, 0.5026548505f);
}

void stg0EBuild307(void) {
    stageBuildFixedCam(0xE, 0, 0.0f, 0.0f, 0.0f, (u32) stg0EPlaces307, 0xC, 0.0f, 20.0f, -75.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 40.0f, 0.6283185482f);
}

void stg2ABuild307(void) {
    gGameWork.geoCacheSize = 40;
    stageBuildFixedCam(0x2A, 0, 0.0f, 0.0f, 0.0f, (u32) stg2APlaces307, 0xA, 145.0f, 20.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, -5.0f, 0.7853981853f);
}

void stg2EBuild307(void) {
    stageBuildFixedCam(0x2E, 0, 0.0f, 0.0f, 77.0f, (u32) stg2EPlaces307, 0xA, -180.0f, 20.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(10.0f, 0.0f, 0.471238941f);
}
