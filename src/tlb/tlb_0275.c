#include "tlb.h"

extern ConnectData area1MeshiAPlacesEne0FAParam[];
extern PersonData area1MeshiAPlacesEne19CParam[];
extern PersonData area1MeshiAPlacesEne1A1Param[];
extern PersonData area1MeshiAPlacesEne1B8Param[];
extern PersonData area1MeshiAPlacesEne1B9Param[];
ConnectData area1MeshiAPlacesEne0FAParam[] = {
    { 15.0f, 25, 0, -966.0f, 0.0f, -130.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData area1MeshiAPlacesEne19CParam[] = {
    { 738, 0x00000000, { 4, 8 }, 0, 0.0f, 0 },
};
PersonData area1MeshiAPlacesEne1A1Param[] = {
    { 739, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
PersonData area1MeshiAPlacesEne1B8Param[] = {
    { 740, 0x00000000, { 5, 8 }, 2, 3.1415927f, 0 },
};
PersonData area1MeshiAPlacesEne1B9Param[] = {
    { 741, 0x00000000, { 5, 8 }, 2, 3.1415927f, 0 },
};
EneDef area1MeshiAPlaces[] = {
    { 0, 40, 0, 105, ENE_CONNECT, 2, (u32) &area1MeshiAPlacesEne0FAParam },
    { 0, 0, -1, 15, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH40A, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH40B, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH40C, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, 15, 17, 70, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -11, 0, 58, ENE_TABLE_GANSO, 2, (u32) &area1MeshiAPlacesEne19CParam },
    { 0, -40, 5, 57, ENE_SHOP_GANSO_ITA, 2, (u32) &area1MeshiAPlacesEne1A1Param },
    { 0, -11, 0, 58, ENE_SHOP_GANSO_GIRL, 0, 0 },
    { 0, 26, 3, 33, ENE_SHOP_KYAKU_DEBU, 2, (u32) &area1MeshiAPlacesEne1B8Param },
    { 0, -27, 5, -11, ENE_SHOP_KYAKU_OBASAN, 2, (u32) &area1MeshiAPlacesEne1B9Param },
};

void area1MeshiAInit(void) {
    stageBuildFixedCam(0xE, 0, 0.0f, 0.0f, 0.0f, (u32) area1MeshiAPlaces, 0xC, 0.0f, 20.0f, -75.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 40.0f, 0.628318548f);
}
