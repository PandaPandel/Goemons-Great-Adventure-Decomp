#include "tlb.h"

/* Archive file 315: the stage 5 town's interiors (gStageList "5 CHA" .. "5 IDO2"). */

extern ConnectData stage5RoomsChaPlacesEne0FAParam[];
extern DoorsData stage5RoomsChaPlacesEne127Param[];
extern u16 stage5RoomsChaPlacesEne19FParamPath[];
extern PersonData stage5RoomsChaPlacesEne19FParam[];
extern ConnectData stage5RoomsHonkePlacesEne0FAParam[];
extern PersonData stage5RoomsHonkePlacesEne105Param[];
extern u32 stage5RoomsHonkePlacesEne1A2ParamPath;
extern PersonData stage5RoomsHonkePlacesEne1A2Param[];
extern u32 stage5RoomsHonkePlacesEne1A1ParamPath;
extern PersonData stage5RoomsHonkePlacesEne1A1Param[];
extern PersonData stage5RoomsHonkePlacesEne1B8Param[];
extern ConnectData stage5RoomsYadoPlacesEne0FAParam[];
extern PersonData stage5RoomsYadoPlacesEne19DParam[];
extern DoorsData stage5RoomsYadoPlacesEne127Param[];
extern u16 stage5RoomsYadoPlacesEne1BAParamPath[];
extern PersonData stage5RoomsYadoPlacesEne1BAParam[];
extern ConnectData stage5RoomsYorozuPlacesEne0FAParam0[];
extern ConnectData stage5RoomsYorozuPlacesEne0FAParam1[];
extern PersonData stage5RoomsYorozuPlacesEne104Param[];
extern PersonData stage5RoomsYorozuPlacesEne12DParam[];
extern ConnectData stage5RoomsUranaiPlacesEne0FAParam0[];
extern ConnectData stage5RoomsUranaiPlacesEne0FAParam1[];
extern ConnectData stage5RoomsUranaiPlacesEne0FAParam2[];
extern PersonData stage5RoomsUranaiPlacesEne128Param[];
extern ConnectData stage5RoomsGansoPlacesEne0FAParam[];
extern PersonData stage5RoomsGansoPlacesEne19CParam[];
extern PersonData stage5RoomsGansoPlacesEne1A1Param[];
extern PersonData stage5RoomsGansoPlacesEne12DParam[];
extern ConnectData stage5RoomsIdo1PlacesEne0FAParam[];
extern u16 stage5RoomsIdo1PlacesEne1BAParamPath[];
extern PersonData stage5RoomsIdo1PlacesEne1BAParam[];
extern ConnectData stage5RoomsIdo2PlacesEne0FAParam[];
extern PersonData stage5RoomsIdo2PlacesEne196Param1[];
extern PersonData stage5RoomsIdo2PlacesEne196Param2[];
ConnectData stage5RoomsChaPlacesEne0FAParam[] = {
    { 10.0f, 110, 0, -610.0f, 0.0f, 270.0f, -1.2566371f, 0x0000, 0, 8 },
};
DoorsData stage5RoomsChaPlacesEne127Param[] = {
    { 0.0f, 10 },
};
u16 stage5RoomsChaPlacesEne19FParamPath[] = { 0x0002, 0x010E, 0x0001, 0x0000 };
PersonData stage5RoomsChaPlacesEne19FParam[] = {
    { 786, (u32) &stage5RoomsChaPlacesEne19FParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stage5RoomsChaPlaces[] = {
    { 0, 32, 0, 100, ENE_CONNECT, 2, (u32) &stage5RoomsChaPlacesEne0FAParam },
    { 0, 31, 0, 79, ENE_DOORS_SET_47, 2, (u32) &stage5RoomsChaPlacesEne127Param },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -50, 18, -12, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 40, 0, 2, ENE_SHOP_CHA_BABA, 2, (u32) &stage5RoomsChaPlacesEne19FParam },
    { 0, 0, 1, 0, ENE_CHAMISE, 0, 0 },
};
ConnectData stage5RoomsHonkePlacesEne0FAParam[] = {
    { 15.0f, 110, 0, -205.0f, 0.0f, 635.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stage5RoomsHonkePlacesEne105Param[] = {
    { 783, 0x00000000, { 4, 8 }, 2, 0.0f, 0 },
};
u32 stage5RoomsHonkePlacesEne1A2ParamPath = 0x00010000;
PersonData stage5RoomsHonkePlacesEne1A2Param[] = {
    { 785, (u32) &stage5RoomsHonkePlacesEne1A2ParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
u32 stage5RoomsHonkePlacesEne1A1ParamPath = 0x00010000;
PersonData stage5RoomsHonkePlacesEne1A1Param[] = {
    { 784, (u32) &stage5RoomsHonkePlacesEne1A1ParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
PersonData stage5RoomsHonkePlacesEne1B8Param[] = {
    { 706, 0x00000000, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef stage5RoomsHonkePlaces[] = {
    { 0, 36, 0, 96, ENE_CONNECT, 2, (u32) &stage5RoomsHonkePlacesEne0FAParam },
    { 0, 59, 0, 96, ENE_CONNECT, 2, (u32) &stage5RoomsHonkePlacesEne0FAParam },
    { 0, 82, 0, 96, ENE_CONNECT, 2, (u32) &stage5RoomsHonkePlacesEne0FAParam },
    { 0, 0, -1, 20, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH45D, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH45E, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH45F, 0, 0 },
    { 0, -64, 0, 39, ENE_SHOP_GANSO_ITA, 2, (u32) &stage5RoomsHonkePlacesEne1A1Param },
    { 0, 55, 0, 5, ENE_SHOP_HONKE_GIRL, 2, (u32) &stage5RoomsHonkePlacesEne1A2Param },
    { 0, 0, 0, 0, ENE_SHOP_BABA, 0, 0 },
    { 0, -75, 7, -22, ENE_TABLE_HONKE, 2, (u32) &stage5RoomsHonkePlacesEne105Param },
    { 0, 56, 7, -30, ENE_SHOP_KYAKU_DEBU, 2, (u32) &stage5RoomsHonkePlacesEne1B8Param },
};
ConnectData stage5RoomsYadoPlacesEne0FAParam[] = {
    { 10.0f, 110, 0, 253.0f, 0.0f, 596.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stage5RoomsYadoPlacesEne19DParam[] = {
    { 750, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
DoorsData stage5RoomsYadoPlacesEne127Param[] = {
    { 0.0f, 13 },
};
u16 stage5RoomsYadoPlacesEne1BAParamPath[] = { 0x0000, 0x000D, 0x0020, 0x0001, 0x0004, 0x003C, 0x0002, 0x00B4, 0x0000, 0x000D, 0xFFF6, 0x0001, 0x0002, 0x005A, 0x0004, 0x003C, 0x0002, 0x0000, 0x0001, 0x0000 };
PersonData stage5RoomsYadoPlacesEne1BAParam[] = {
    { 703, (u32) &stage5RoomsYadoPlacesEne1BAParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef stage5RoomsYadoPlaces[] = {
    { 0, 60, 0, 70, ENE_CONNECT, 2, (u32) &stage5RoomsYadoPlacesEne0FAParam },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -29, 27, 6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, 60, 0, 48, ENE_DOORS_SET_47, 2, (u32) &stage5RoomsYadoPlacesEne127Param },
    { 0, 0, 0, 0, ENE_SHOP_YADOYA, 0, 0 },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 57, 4, -32, ENE_TABLE_YADOYA, 2, (u32) &stage5RoomsYadoPlacesEne19DParam },
    { 0, 13, 0, 32, ENE_SHOP_KYAKU_BOUZU, 2, (u32) &stage5RoomsYadoPlacesEne1BAParam },
};
ConnectData stage5RoomsYorozuPlacesEne0FAParam0[] = {
    { 15.0f, 110, 0, 591.0f, 0.0f, 293.0f, 0.9424778f, 0x0000, 0, 0 },
};
ConnectData stage5RoomsYorozuPlacesEne0FAParam1[] = {
    { 15.0f, 110, 0, 560.0f, 0.0f, 345.0f, 0.9424778f, 0x0000, 0, 0 },
};
PersonData stage5RoomsYorozuPlacesEne104Param[] = {
    { 780, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
PersonData stage5RoomsYorozuPlacesEne12DParam[] = {
    { 709, 0x00000000, { 5, 8 }, 2, 1.2566371f, 1 },
};
EneDef stage5RoomsYorozuPlaces[] = {
    { 0, 45, 0, 105, ENE_CONNECT, 2, (u32) &stage5RoomsYorozuPlacesEne0FAParam0 },
    { 0, -15, 0, 105, ENE_CONNECT, 2, (u32) &stage5RoomsYorozuPlacesEne0FAParam1 },
    { 0, 0, -1, 30, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_YOROZU_DEBU, 0, 0 },
    { 0, 19, 10, -15, ENE_TABLE_YOROZUYA, 2, (u32) &stage5RoomsYorozuPlacesEne104Param },
    { 0, 44, 13, -13, ENE_SHOP_YOROZU_ITEM4, 2, 0 },
    { 0, 59, 13, -13, ENE_SHOP_YOROZU_ITEM3, 2, 0 },
    { 0, 74, 13, -13, ENE_SHOP_YOROZU_KIN_YOROI, 2, 0 },
    { 0, -52, 10, -14, ENE_SHOP_KYAKU_OYAJI, 2, (u32) &stage5RoomsYorozuPlacesEne12DParam },
};
ConnectData stage5RoomsUranaiPlacesEne0FAParam0[] = {
    { 10.0f, 110, 0, -578.0f, 0.0f, 1216.0f, 0.0f, 0x0007, 0, 0 },
};
ConnectData stage5RoomsUranaiPlacesEne0FAParam1[] = {
    { 10.0f, 110, 0, -529.0f, 0.0f, 1237.0f, 0.0f, 0x0007, 0, 0 },
};
ConnectData stage5RoomsUranaiPlacesEne0FAParam2[] = {
    { 10.0f, 110, 0, -478.0f, 0.0f, 1255.0f, 0.0f, 0x0007, 0, 0 },
};
PersonData stage5RoomsUranaiPlacesEne128Param[] = {
    { 43, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef stage5RoomsUranaiPlaces[] = {
    { 0, -40, 10, 135, ENE_CONNECT, 2, (u32) &stage5RoomsUranaiPlacesEne0FAParam0 },
    { 0, 0, 10, 135, ENE_CONNECT, 2, (u32) &stage5RoomsUranaiPlacesEne0FAParam1 },
    { 0, 40, 10, 135, ENE_CONNECT, 2, (u32) &stage5RoomsUranaiPlacesEne0FAParam2 },
    { 0, 0, -1, -80, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_PURA, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_DANCER, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_DANCER, 0, 0 },
    { 0, 0, 0, 0, ENE_URANAI_DECO, 0, 0 },
    { 0, 0, 0, 0, ENE_URANAI_CAMERA, 0, 0 },
    { 0, 40, 5, 0, ENE_URANAI_TABLE, 2, (u32) &stage5RoomsUranaiPlacesEne128Param },
};
EneDef stage5RoomsSecretPlaces[] = {
    { 0, 0, 0, 0, ENE_DTSK_TELE, 0, 0 },
};
ConnectData stage5RoomsGansoPlacesEne0FAParam[] = {
    { 15.0f, 110, 0, 123.0f, 0.0f, 1345.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stage5RoomsGansoPlacesEne19CParam[] = {
    { 781, 0x00000000, { 4, 8 }, 0, 0.0f, 0 },
};
PersonData stage5RoomsGansoPlacesEne1A1Param[] = {
    { 782, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
PersonData stage5RoomsGansoPlacesEne12DParam[] = {
    { 707, 0x00000000, { 5, 8 }, 2, 3.1415927f, 1 },
};
EneDef stage5RoomsGansoPlaces[] = {
    { 0, 40, 0, 105, ENE_CONNECT, 2, (u32) &stage5RoomsGansoPlacesEne0FAParam },
    { 0, 0, -1, 15, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH45A, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH45B, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH45C, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, 15, 17, 70, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -11, 0, 58, ENE_TABLE_GANSO, 2, (u32) &stage5RoomsGansoPlacesEne19CParam },
    { 0, -40, 5, 57, ENE_SHOP_GANSO_ITA, 2, (u32) &stage5RoomsGansoPlacesEne1A1Param },
    { 0, -11, 0, 58, ENE_SHOP_GANSO_GIRL, 0, 0 },
    { 0, 20, 4, -17, ENE_SHOP_KYAKU_OYAJI, 2, (u32) &stage5RoomsGansoPlacesEne12DParam },
};
ConnectData stage5RoomsIdo1PlacesEne0FAParam[] = {
    { 16.0f, 110, 0, -64.0f, 0.0f, 999.0f, 0.0f, 0x0000, 0, 0 },
};
u16 stage5RoomsIdo1PlacesEne1BAParamPath[] = { 0x0000, 0xFFCA, 0x0018, 0x0001, 0x0004, 0x001E, 0x0000, 0xFFCD, 0xFFDD, 0x0001, 0x0004, 0x001E, 0x0001, 0x0000 };
PersonData stage5RoomsIdo1PlacesEne1BAParam[] = {
    { 710, (u32) &stage5RoomsIdo1PlacesEne1BAParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef stage5RoomsIdo1Places[] = {
    { 0, 0, 50, 0, ENE_CONNECT, 2, (u32) &stage5RoomsIdo1PlacesEne0FAParam },
    { 0, -51, 0, -35, ENE_SHOP_KYAKU_BOUZU, 2, (u32) &stage5RoomsIdo1PlacesEne1BAParam },
};
ConnectData stage5RoomsIdo2PlacesEne0FAParam[] = {
    { 16.0f, 110, 0, 502.0f, 0.0f, 840.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stage5RoomsIdo2PlacesEne196Param1[] = {
    { 711, 0x00000000, { 4, 8 }, 3, 0.0f, 0 },
};
PersonData stage5RoomsIdo2PlacesEne196Param2[] = {
    { 712, 0x00000000, { 4, 8 }, 3, 0.0f, 0 },
};
EneDef stage5RoomsIdo2Places[] = {
    { 0, 0, 50, 0, ENE_CONNECT, 2, (u32) &stage5RoomsIdo2PlacesEne0FAParam },
    { 0, -30, 12, 53, ENE_TOKAGE, 2, (u32) &stage5RoomsIdo2PlacesEne196Param1 },
    { 0, -30, 2, -32, ENE_TOKAGE, 2, (u32) &stage5RoomsIdo2PlacesEne196Param2 },
};
extern u8 stage5RoomsBuildSecretColl[];

/* "5 CHA" */
void stage5RoomsBuildCha(void) {
    stageBuildFixedCam(0xD, 0, 0.0f, 0.0f, 0.0f, (u32) stage5RoomsChaPlaces, 7, 0.0f, 20.0f, -75.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, -40.0f, 0.6283185482f);
    camSunSetTunes4(0xBE, 0xBB, 0xBC, 0xBD);
    camSunSetDawnSound(0x42F);
}

/* "5 HONKE" */
void stage5RoomsBuildHonke(void) {
    stageBuildFixedCam(0x20, 0, 0.0f, 0.0f, 0.0f, (u32) stage5RoomsHonkePlaces, 0xC, 138.0f, 20.0f, 45.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(50.0f, 10.0f, 0.5969026041f);
}

/* "5 YADO" */
void stage5RoomsBuildYado(void) {
    stageBuildFixedCam(0x10, 0, 0.0f, 0.0f, 0.0f, (u32) stage5RoomsYadoPlaces, 8, 120.0f, 20.0f, 20.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(40.0f, 5.0f, 0.5026548505f);
}

/* "5 YOROZU" */
void stage5RoomsBuildYorozu(void) {
    stageBuildFixedCam(0xF, 0, 0.0f, 0.0f, 0.0f, (u32) stage5RoomsYorozuPlaces, 9, 120.0f, 20.0f, 50.0f);
    camAiSetFar(600);
    camAiSetTurnLimit(40.0f, 36.0f, 0.471238941f);
}

/* "5 URANAI" */
void stage5RoomsBuildUranai(void) {
    stageBuildFixedCam(0x2E, 0, 0.0f, 0.0f, 77.0f, (u32) stage5RoomsUranaiPlaces, 0xA, -180.0f, 20.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(10.0f, 0.0f, 0.471238941f);
}

/* "5 ?" */
void stage5RoomsBuildSecret(void) {
    floadLoadSet(0xDF);
    vramCreate(gStageTask);
    vramLoadColl((u32) stage5RoomsBuildSecretColl, floadGetSetFile(0xDF, 0));
    stageBuildPlain(0, 0.0f, 0.0f, 0.0f, (u32) stage5RoomsSecretPlaces, 1);
    camAiSetFar(1000);
    eneMake(ENE_DTSK_TELE, 0, 0.0f, 0.0f, 0.0f);
    sndPlay(0x2C);
}

/* "5 GANSO" */
void stage5RoomsBuildGanso(void) {
    stageBuildFixedCam(0xE, 0, 0.0f, 0.0f, 0.0f, (u32) stage5RoomsGansoPlaces, 0xB, 0.0f, 20.0f, -75.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 40.0f, 0.6283185482f);
}

/* "5 IDO1" */
void stage5RoomsBuildIdo1(void) {
    stageBuildFixedCam(0x44, 0, 0.0f, 10.0f, 0.0f, (u32) stage5RoomsIdo1Places, 2, 85.0f, 25.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(10.0f, 0.0f, 0.6283185482f);
    sndSetReverb(1, 15);
}

/* "5 IDO2" */
void stage5RoomsBuildIdo2(void) {
    stageBuildFixedCam(0x44, 0, 0.0f, 10.0f, 0.0f, (u32) stage5RoomsIdo2Places, 3, 85.0f, 25.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(10.0f, 0.0f, 0.6283185482f);
    sndSetReverb(1, 15);
}
