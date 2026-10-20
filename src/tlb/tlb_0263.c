#include "tlb.h"

extern ConnectData wiseman2RoomPlacesEne0FAParam[];
extern ExitData wiseman2RoomPlacesEne0F8Param[];
extern DoorsData wiseman2RoomPlacesEne127Param5[];
extern DoorsData wiseman2RoomPlacesEne127Param6[];
extern ConnectData wiseman2LaboPlacesEne0FAParam[];
extern DoorsData wiseman2LaboPlacesEne127Param[];
extern u16 wiseman2LaboPlacesEne103ParamPath[];
extern PersonData wiseman2LaboPlacesEne103Param[];
ConnectData wiseman2RoomPlacesEne0FAParam[] = {
    { 15.0f, 131, 0, 0.0f, 0.0f, -80.0f, 0.0f, 0x0000, 0, 0 },
};
ExitData wiseman2RoomPlacesEne0F8Param[] = {
    { 12.0f, 0, 0, 0, 0 },
};
DoorsData wiseman2RoomPlacesEne127Param5[] = {
    { 0.0f, 11 },
};
DoorsData wiseman2RoomPlacesEne127Param6[] = {
    { -1.5707964f, 12 },
};
EneDef wiseman2RoomPlaces[] = {
    { 0, -15, 12, 133, ENE_CONNECT, 2, (u32) &wiseman2RoomPlacesEne0FAParam },
    { 0, 15, 12, 133, ENE_CONNECT, 2, (u32) &wiseman2RoomPlacesEne0FAParam },
    { 0, -132, 10, -78, ENE_EXIT, 2, (u32) &wiseman2RoomPlacesEne0F8Param },
    { 0, -132, 10, -102, ENE_EXIT, 2, (u32) &wiseman2RoomPlacesEne0F8Param },
    { 0, 0, 1, 0, ENE_WISEMAN_ROOM, 0, 0 },
    { 0, 0, 2, 122, ENE_DOORS_SET_47, 2, (u32) &wiseman2RoomPlacesEne127Param5 },
    { 0, -122, 0, -92, ENE_DOORS_SET_47, 2, (u32) &wiseman2RoomPlacesEne127Param6 },
    { 0, 0, -1, 0, ENE_SHOP_ROOM_BG, 0, 0 },
};
ConnectData wiseman2LaboPlacesEne0FAParam[] = {
    { 15.0f, 130, 0, 0.0f, 0.0f, 114.0f, 3.4557521f, 0x0000, 0, 0 },
};
DoorsData wiseman2LaboPlacesEne127Param[] = {
    { 3.1415927f, 11 },
};
u16 wiseman2LaboPlacesEne103ParamPath[] = { 0x0000, 0x0000, 0x0028, 0x0002, 0x0000, 0xFFEC, 0x0014, 0x0002, 0x0000, 0x0000, 0x0000, 0x0002, 0x0000, 0x0014, 0x0014, 0x0002, 0x0001, 0x0000 };
PersonData wiseman2LaboPlacesEne103Param[] = {
    { 34, (u32) &wiseman2LaboPlacesEne103ParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef wiseman2LaboPlaces[] = {
    { 0, -15, 0, -106, ENE_CONNECT, 2, (u32) &wiseman2LaboPlacesEne0FAParam },
    { 0, 15, 0, -106, ENE_CONNECT, 2, (u32) &wiseman2LaboPlacesEne0FAParam },
    { 0, 0, 1, 0, ENE_WISEMAN_ROOM2, 0, 0 },
    { 0, 20, 0, 20, ENE_ETSK_WISEMAN_MAIN, 2, (u32) &wiseman2LaboPlacesEne103Param },
    { 0, 0, 1, -90, ENE_DOORS_SET_47, 2, (u32) &wiseman2LaboPlacesEne127Param },
};

void wiseman2RoomInit(void) {
    stageBuildFixedCam(0x5F, 0, 0.0f, 0.0f, 0.0f, (u32) wiseman2RoomPlaces, 8, 0.0f, 30.0f, -150.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 0.0f, 0.753982246f);
}

void wiseman2LaboInit(void) {
    stageBuildFixedCam(0x60, 0, 0.0f, 0.0f, 0.0f, (u32) wiseman2LaboPlaces, 5, 115.0f, 30.0f, -60.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(30.0f, -22.0f, 0.471238941f);
}
