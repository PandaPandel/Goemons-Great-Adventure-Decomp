#include "tlb.h"

extern ConnectData stage2MeshiAPlacesEne0FAParam[];
extern PersonData stage2MeshiAPlacesEne19CParam[];
extern PersonData stage2MeshiAPlacesEne1A1Param[];
extern PersonData stage2MeshiAPlacesEne1B9Param[];
extern u16 stage2MeshiAPlacesEne1BBParamPath[];
extern PersonData stage2MeshiAPlacesEne1BBParam[];
ConnectData stage2MeshiAPlacesEne0FAParam[] = {
    { 15.0f, 50, 0, -648.0f, 0.0f, 781.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stage2MeshiAPlacesEne19CParam[] = {
    { 758, 0x00000000, { 4, 8 }, 0, 0.0f, 0 },
};
PersonData stage2MeshiAPlacesEne1A1Param[] = {
    { 759, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
PersonData stage2MeshiAPlacesEne1B9Param[] = {
    { 525, 0x00000000, { 5, 8 }, 2, 3.1415927f, 1 },
};
u16 stage2MeshiAPlacesEne1BBParamPath[] = { 0x0004, 0x001E, 0x0000, 0xFFDC, 0x000D, 0x0003, 0x0004, 0x001E, 0x0000, 0xFFF1, 0x000D, 0x0003, 0x0001, 0x0000 };
PersonData stage2MeshiAPlacesEne1BBParam[] = {
    { 524, (u32) &stage2MeshiAPlacesEne1BBParamPath, { 5, 8 }, 2, 3.1415927f, 1 },
};
EneDef stage2MeshiAPlaces[] = {
    { 0, 40, 0, 105, ENE_CONNECT, 2, (u32) &stage2MeshiAPlacesEne0FAParam },
    { 0, 0, -1, 15, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH41A, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH41B, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MANPUKU_DISH41C, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, 15, 17, 70, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -11, 0, 58, ENE_TABLE_GANSO, 2, (u32) &stage2MeshiAPlacesEne19CParam },
    { 0, -40, 5, 57, ENE_SHOP_GANSO_ITA, 2, (u32) &stage2MeshiAPlacesEne1A1Param },
    { 0, -11, 0, 58, ENE_SHOP_GANSO_GIRL, 0, 0 },
    { 0, 26, 3, 33, ENE_SHOP_KYAKU_OBASAN, 2, (u32) &stage2MeshiAPlacesEne1B9Param },
    { 0, -27, 0, -11, ENE_SHOP_KYAKU_OTOKO, 2, (u32) &stage2MeshiAPlacesEne1BBParam },
};

void stage2MeshiAInit(void) {
    stageBuildFixedCam(0xE, 0, 0.0f, 0.0f, 0.0f, (u32) stage2MeshiAPlaces, 12, 0.0f, 20.0f, -75.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(0.0f, 40.0f, 0.6283185482f);
}
