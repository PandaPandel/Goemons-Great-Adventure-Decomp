#include "tlb.h"

extern ConnectData area1MeshiBPlacesEne0FAParam[];
extern PersonData area1MeshiBPlacesEne105Param[];
extern PersonData area1MeshiBPlacesEne1A2Param[];
extern PersonData area1MeshiBPlacesEne1A1Param[];
extern PersonData area1MeshiBPlacesEne12DParam[];
ConnectData area1MeshiBPlacesEne0FAParam[] = {
    { 15.0f, 25, 0, -706.0f, 0.0f, -120.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData area1MeshiBPlacesEne105Param[] = {
    { 742, 0x00000000, { 4, 8 }, 2, 0.0f, 0 },
};
PersonData area1MeshiBPlacesEne1A2Param[] = {
    { 744, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
PersonData area1MeshiBPlacesEne1A1Param[] = {
    { 743, 0x00000000, { 5, 8 }, 2, 1.5707964f, 0 },
};
PersonData area1MeshiBPlacesEne12DParam[] = {
    { 745, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef area1MeshiBPlaces[] = {
    { 0, 36, 0, 96, ENE_CONNECT, 2, (u32) &area1MeshiBPlacesEne0FAParam },
    { 0, 59, 0, 96, ENE_CONNECT, 2, (u32) &area1MeshiBPlacesEne0FAParam },
    { 0, 82, 0, 96, ENE_CONNECT, 2, (u32) &area1MeshiBPlacesEne0FAParam },
    { 0, 0, -1, 20, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH40D, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH40E, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH40F, 0, 0 },
    { 0, -64, 0, 39, ENE_SHOP_GANSO_ITA, 2, (u32) &area1MeshiBPlacesEne1A1Param },
    { 0, 55, 0, 5, ENE_SHOP_HONKE_GIRL, 2, (u32) &area1MeshiBPlacesEne1A2Param },
    { 0, 0, 0, 0, ENE_SHOP_BABA, 0, 0 },
    { 0, -75, 7, -22, ENE_TABLE_HONKE, 2, (u32) &area1MeshiBPlacesEne105Param },
    { 0, 25, 7, -33, ENE_SHOP_KYAKU_OYAJI, 2, (u32) &area1MeshiBPlacesEne12DParam },
};

void area1MeshiBInit(void) {
    stageBuildFixedCam(0x20, 0, 0.0f, 0.0f, 0.0f, (u32) area1MeshiBPlaces, 0xC, 138.0f, 20.0f, 45.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(50.0f, 10.0f, 0.596902609f);
}
