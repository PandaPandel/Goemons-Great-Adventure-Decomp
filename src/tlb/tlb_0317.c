#include "tlb.h"

/* Archive file 317: the stage 4 town's interiors (gStageList "4 CHA" .. "4 URANAI"). */

extern ConnectData stage4RoomsChaPlacesEne0FAParam[];
extern DoorsData stage4RoomsChaPlacesEne127Param[];
extern u16 stage4RoomsChaPlacesEne19FParamPath[];
extern PersonData stage4RoomsChaPlacesEne19FParam[];
extern ConnectData stage4RoomsYoro1PlacesEne0FAParam[];
extern PersonData stage4RoomsYoro1PlacesEne104Param[];
extern u16 stage4RoomsYoro1PlacesEne1BAParamPath[];
extern PersonData stage4RoomsYoro1PlacesEne1BAParam[];
extern ConnectData stage4RoomsGansoPlacesEne0FAParam[];
extern PersonData stage4RoomsGansoPlacesEne19CParam[];
extern PersonData stage4RoomsGansoPlacesEne1A1Param[];
extern PersonData stage4RoomsGansoPlacesEne1B8Param[];
extern PersonData stage4RoomsGansoPlacesEne1B9Param[];
extern ConnectData stage4RoomsYado1PlacesEne0FAParam[];
extern PersonData stage4RoomsYado1PlacesEne19DParam[];
extern DoorsData stage4RoomsYado1PlacesEne127Param[];
extern u16 stage4RoomsYado1PlacesEne12DParamPath[];
extern PersonData stage4RoomsYado1PlacesEne12DParam[];
extern ConnectData stage4RoomsYoro2PlacesEne0FAParam[];
extern PersonData stage4RoomsYoro2PlacesEne104Param[];
extern u32 stage4RoomsYoro2UnusedPersonPath1[];
extern u32 stage4RoomsYoro2UnusedPersonPath2[];
extern u32 stage4RoomsYoro2UnusedPersonPath3[];
extern ConnectData stage4RoomsYado2PlacesEne0FAParam[];
extern PersonData stage4RoomsYado2PlacesEne19DParam[];
extern u16 stage4RoomsYado2PlacesEne1B9ParamPath[];
extern PersonData stage4RoomsYado2PlacesEne1B9Param[];
extern ConnectData stage4RoomsUranaiPlacesEne0FAParam[];
extern PersonData stage4RoomsUranaiPlacesEne128Param[];
ConnectData stage4RoomsChaPlacesEne0FAParam[] = {
    { 10.0f, 95, 0, 76.0f, 0.0f, -12.0f, 0.0f, 0x0000, 0, 8 },
};
DoorsData stage4RoomsChaPlacesEne127Param[] = {
    { 0.0f, 10 },
};
u16 stage4RoomsChaPlacesEne19FParamPath[] = { 0x0002, 0x010E, 0x0001, 0x0000 };
PersonData stage4RoomsChaPlacesEne19FParam[] = {
    { 779, (u32) &stage4RoomsChaPlacesEne19FParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stage4RoomsChaPlaces[] = {
    { 0, 32, 0, 100, ENE_CONNECT, 2, (u32) &stage4RoomsChaPlacesEne0FAParam },
    { 0, 31, 0, 79, ENE_DOORS_SET_47, 2, (u32) &stage4RoomsChaPlacesEne127Param },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -50, 18, -12, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 40, 0, 2, ENE_SHOP_CHA_BABA, 2, (u32) &stage4RoomsChaPlacesEne19FParam },
    { 0, 0, 1, 0, ENE_CHAMISE, 0, 0 },
};
ConnectData stage4RoomsYoro1PlacesEne0FAParam[] = {
    { 15.0f, 96, 0, -200.0f, 0.0f, -25.0f, 0.31415927f, 0x0000, 0, 0 },
};
PersonData stage4RoomsYoro1PlacesEne104Param[] = {
    { 775, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
u16 stage4RoomsYoro1PlacesEne1BAParamPath[] = { 0x0000, 0xFFCE, 0x0016, 0x0003, 0x0004, 0x001E, 0x0002, 0x005A, 0x0000, 0x003C, 0x0016, 0x0003, 0x0002, 0x00B4, 0x0004, 0x0096, 0x0002, 0x010E, 0x0001, 0x0000 };
PersonData stage4RoomsYoro1PlacesEne1BAParam[] = {
    { 651, (u32) &stage4RoomsYoro1PlacesEne1BAParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stage4RoomsYoro1Places[] = {
    { 0, 45, 0, 105, ENE_CONNECT, 2, (u32) &stage4RoomsYoro1PlacesEne0FAParam },
    { 0, -15, 0, 105, ENE_CONNECT, 2, (u32) &stage4RoomsYoro1PlacesEne0FAParam },
    { 0, 0, -1, 30, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_YOROZU_DEBU, 0, 0 },
    { 0, 19, 10, -15, ENE_TABLE_YOROZUYA, 2, (u32) &stage4RoomsYoro1PlacesEne104Param },
    { 0, 44, 13, -13, ENE_SHOP_YOROZU_ITEM1, 2, 0 },
    { 0, 59, 13, -13, ENE_SHOP_YOROZU_ITEM2, 2, 0 },
    { 0, 74, 13, -13, ENE_SHOP_YOROZU_ITEM3, 2, 0 },
    { 0, 0, 1, 0, ENE_SHOP_KYAKU_BOUZU, 2, (u32) &stage4RoomsYoro1PlacesEne1BAParam },
};
ConnectData stage4RoomsGansoPlacesEne0FAParam[] = {
    { 15.0f, 96, 0, 224.0f, 0.0f, -4.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stage4RoomsGansoPlacesEne19CParam[] = {
    { 777, 0x00000000, { 4, 8 }, 0, 0.0f, 0 },
};
PersonData stage4RoomsGansoPlacesEne1A1Param[] = {
    { 778, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
PersonData stage4RoomsGansoPlacesEne1B8Param[] = {
    { 654, 0x00000000, { 5, 8 }, 2, 3.1415927f, 0 },
};
PersonData stage4RoomsGansoPlacesEne1B9Param[] = {
    { 653, 0x00000000, { 5, 8 }, 2, 3.1415927f, 0 },
};
EneDef stage4RoomsGansoPlaces[] = {
    { 0, 40, 0, 105, ENE_CONNECT, 2, (u32) &stage4RoomsGansoPlacesEne0FAParam },
    { 0, 0, -1, 15, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH43A, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH43B, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH43C, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, 15, 17, 70, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -11, 0, 58, ENE_TABLE_GANSO, 2, (u32) &stage4RoomsGansoPlacesEne19CParam },
    { 0, -40, 5, 57, ENE_SHOP_GANSO_ITA, 2, (u32) &stage4RoomsGansoPlacesEne1A1Param },
    { 0, -11, 0, 58, ENE_SHOP_GANSO_GIRL, 0, 0 },
    { 0, 26, 3, 33, ENE_SHOP_KYAKU_DEBU, 2, (u32) &stage4RoomsGansoPlacesEne1B8Param },
    { 0, -27, 5, -11, ENE_SHOP_KYAKU_OBASAN, 2, (u32) &stage4RoomsGansoPlacesEne1B9Param },
};
ConnectData stage4RoomsYado1PlacesEne0FAParam[] = {
    { 10.0f, 97, 0, 0.0f, 0.0f, 44.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stage4RoomsYado1PlacesEne19DParam[] = {
    { 750, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
DoorsData stage4RoomsYado1PlacesEne127Param[] = {
    { 0.0f, 13 },
};
u16 stage4RoomsYado1PlacesEne12DParamPath[] = { 0x0004, 0x003C, 0x0002, 0x00B4, 0x0001, 0x0000 };
PersonData stage4RoomsYado1PlacesEne12DParam[] = {
    { 656, (u32) &stage4RoomsYado1PlacesEne12DParamPath, { 5, 8 }, 2, 1.374449f, 0 },
};
EneDef stage4RoomsYado1Places[] = {
    { 0, 60, 0, 70, ENE_CONNECT, 2, (u32) &stage4RoomsYado1PlacesEne0FAParam },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -29, 27, 6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 60, 0, 48, ENE_DOORS_SET_47, 2, (u32) &stage4RoomsYado1PlacesEne127Param },
    { 0, 0, 0, 0, ENE_SHOP_YADOYA, 0, 0 },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 57, 4, -32, ENE_TABLE_YADOYA, 2, (u32) &stage4RoomsYado1PlacesEne19DParam },
    { 0, -5, 10, -4, ENE_SHOP_KYAKU_OYAJI, 2, (u32) &stage4RoomsYado1PlacesEne12DParam },
};
ConnectData stage4RoomsYoro2PlacesEne0FAParam[] = {
    { 15.0f, 97, 0, 88.0f, 0.0f, 3.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stage4RoomsYoro2PlacesEne104Param[] = {
    { 776, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
u32 stage4RoomsYoro2UnusedPersonPath1[] = { 0x0000FFCE, 0x00160003, 0x0004001E, 0x0002005A, 0x0000003C, 0x00160003, 0x000200B4, 0x00040096, 0x0002010E, 0x00010000, 0x000002FF, (u32) &stage4RoomsYoro2UnusedPersonPath1, 0x05080200, 0x00000000, 0x00000000 };
u32 stage4RoomsYoro2UnusedPersonPath2[] = { 0x0004001E, 0x0002005A, 0x00010000, 0x00000291, (u32) &stage4RoomsYoro2UnusedPersonPath2, 0x05080200, 0x00000000, 0x00000000 };
u32 stage4RoomsYoro2UnusedPersonPath3[] = { 0x0004001E, 0x0002005A, 0x00010000, 0x00000292, (u32) &stage4RoomsYoro2UnusedPersonPath3, 0x05080200, 0x00000000, 0x00000000 };
EneDef stage4RoomsYoro2Places[] = {
    { 0, 45, 0, 105, ENE_CONNECT, 2, (u32) &stage4RoomsYoro2PlacesEne0FAParam },
    { 0, -15, 0, 105, ENE_CONNECT, 2, (u32) &stage4RoomsYoro2PlacesEne0FAParam },
    { 0, 0, -1, 30, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_YOROZU_DEBU, 0, 0 },
    { 0, 19, 10, -15, ENE_TABLE_YOROZUYA, 2, (u32) &stage4RoomsYoro2PlacesEne104Param },
    { 0, 44, 13, -13, ENE_SHOP_YOROZU_ITEM4, 2, 0 },
    { 0, 59, 13, -13, ENE_SHOP_YOROZU_ITEM0, 2, 0 },
    { 0, 74, 13, -13, ENE_SHOP_YOROZU_KIN_YOROI, 2, 0 },
};
ConnectData stage4RoomsYado2PlacesEne0FAParam[] = {
    { 10.0f, 111, 0, -245.0f, 167.0f, 181.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stage4RoomsYado2PlacesEne19DParam[] = {
    { 750, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
u16 stage4RoomsYado2PlacesEne1B9ParamPath[] = { 0x0000, 0x000D, 0x0020, 0x0001, 0x0004, 0x003C, 0x0002, 0x00B4, 0x0000, 0x000D, 0xFFF6, 0x0001, 0x0002, 0x005A, 0x0004, 0x003C, 0x0002, 0x0000, 0x0001, 0x0000 };
PersonData stage4RoomsYado2PlacesEne1B9Param[] = {
    { 659, (u32) &stage4RoomsYado2PlacesEne1B9ParamPath, { 5, 8 }, 2, 2.877754f, 0 },
};
EneDef stage4RoomsYado2Places[] = {
    { 0, 60, 0, 70, ENE_CONNECT, 2, (u32) &stage4RoomsYado2PlacesEne0FAParam },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -29, 27, 6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 60, 0, 48, ENE_DOORS_SET_47, 2, (u32) &stage4RoomsYado1PlacesEne127Param },
    { 0, 0, 0, 0, ENE_SHOP_YADOYA, 0, 0 },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 57, 4, -32, ENE_TABLE_YADOYA, 2, (u32) &stage4RoomsYado2PlacesEne19DParam },
    { 0, -20, 10, 15, ENE_SHOP_KYAKU_OBASAN, 2, (u32) &stage4RoomsYado2PlacesEne1B9Param },
};
ConnectData stage4RoomsUranaiPlacesEne0FAParam[] = {
    { 10.0f, 112, 0, 0.0f, 10.0f, -115.0f, 3.1415927f, 0x0007, 0, 0 },
};
PersonData stage4RoomsUranaiPlacesEne128Param[] = {
    { 42, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stage4RoomsUranaiPlaces[] = {
    { 0, -40, 10, 135, ENE_CONNECT, 2, (u32) &stage4RoomsUranaiPlacesEne0FAParam },
    { 0, 0, 10, 135, ENE_CONNECT, 2, (u32) &stage4RoomsUranaiPlacesEne0FAParam },
    { 0, 40, 10, 135, ENE_CONNECT, 2, (u32) &stage4RoomsUranaiPlacesEne0FAParam },
    { 0, 0, -1, -80, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_PURA, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_DANCER, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_DANCER, 0, 0 },
    { 0, 0, 0, 0, ENE_URANAI_DECO, 0, 0 },
    { 0, 0, 0, 0, ENE_URANAI_CAMERA, 0, 0 },
    { 0, 40, 5, 0, ENE_URANAI_TABLE, 2, (u32) &stage4RoomsUranaiPlacesEne128Param },
};

/* "4 CHA" */
void stage4RoomsBuildCha(void) {
    stageBuildFixedCam(0xD, 0, 0.0f, 0.0f, 0.0f, (u32) stage4RoomsChaPlaces, 7, 0.0f, 20.0f, -75.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, -40.0f, 0.6283185482f);
    camSunSetTunes4(0x89, 0x86, 0x87, 0x88);
    camSunSetDawnSound(0x42F);
}

/* "4 YORO1" */
void stage4RoomsBuildYoro1(void) {
    stageBuildFixedCam(0xF, 0, 0.0f, 0.0f, 0.0f, (u32) stage4RoomsYoro1Places, 9, 120.0f, 20.0f, 50.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, 36.0f, 0.471238941f);
}

/* "4 GANSO" */
void stage4RoomsBuildGanso(void) {
    stageBuildFixedCam(0xE, 0, 0.0f, 0.0f, 0.0f, (u32) stage4RoomsGansoPlaces, 0xC, 0.0f, 20.0f, -75.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 40.0f, 0.6283185482f);
}

/* "4 YADO1" */
void stage4RoomsBuildYado1(void) {
    stageBuildFixedCam(0x10, 0, 0.0f, 0.0f, 0.0f, (u32) stage4RoomsYado1Places, 8, 120.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, 5.0f, 0.5026548505f);
}

/* "4 YORO2" */
void stage4RoomsBuildYoro2(void) {
    stageBuildFixedCam(0xF, 0, 0.0f, 0.0f, 0.0f, (u32) stage4RoomsYoro2Places, 8, 120.0f, 20.0f, 50.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, 36.0f, 0.471238941f);
}

/* "4 YADO2" */
void stage4RoomsBuildYado2(void) {
    stageBuildFixedCam(0x10, 0, 0.0f, 0.0f, 0.0f, (u32) stage4RoomsYado2Places, 8, 120.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, 5.0f, 0.5026548505f);
}

/* "4 URANAI" */
void stage4RoomsBuildUranai(void) {
    stageBuildFixedCam(0x2E, 0, 0.0f, 0.0f, 77.0f, (u32) stage4RoomsUranaiPlaces, 0xA, -180.0f, 20.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(10.0f, 0.0f, 0.471238941f);
}
