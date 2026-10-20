#include "tlb.h"

extern ConnectData stage1MituyaPlacesEne0FAParam[];
extern DoorsData stage1MituyaPlacesEne127Param[];
extern u16 stage1MituyaPlacesEne19FParamPath[];
extern PersonData stage1MituyaPlacesEne19FParam[];
extern u16 stage1MituyaPlacesEne138ParamPath[];
extern PersonData stage1MituyaPlacesEne138Param[];
extern u16 stage1MituyaPlacesEne130ParamPath[];
extern PersonData stage1MituyaPlacesEne130Param[];
ConnectData stage1MituyaPlacesEne0FAParam[] = {
    { 10.0f, 25, 0, 908.0f, 0.0f, 202.0f, -0.9424778f, 0x0000, 0, 0 },
};
DoorsData stage1MituyaPlacesEne127Param[] = {
    { 0.0f, 14 },
};
u16 stage1MituyaPlacesEne19FParamPath[] = { 0x0000, 0xFFE0, 0xFFD8, 0x0001, 0x0002, 0x010E, 0x0004, 0x003C, 0x0002, 0x0000, 0x0000, 0xFFE0, 0xFFF4, 0x0001, 0x0002, 0x010E, 0x0004, 0x003C, 0x0002, 0x00B4, 0x0001, 0x0000 };
PersonData stage1MituyaPlacesEne19FParam[] = {
    { 481, (u32) &stage1MituyaPlacesEne19FParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
u16 stage1MituyaPlacesEne138ParamPath[] = { 0x0002, 0x005A, 0x0004, 0x003C, 0x0002, 0x00B4, 0x0004, 0x003C, 0x0001, 0x0000 };
PersonData stage1MituyaPlacesEne138Param[] = {
    { 482, (u32) &stage1MituyaPlacesEne138ParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
u16 stage1MituyaPlacesEne130ParamPath[] = { 0x0002, 0x00B4, 0x0004, 0x003C, 0x0002, 0x010E, 0x0004, 0x003C, 0x0001, 0x0000 };
PersonData stage1MituyaPlacesEne130Param[] = {
    { 483, (u32) &stage1MituyaPlacesEne130ParamPath, { 5, 8 }, 2, 0.0f, 1 },
};
EneDef stage1MituyaPlaces[] = {
    { 0, 10, 5, 68, ENE_CONNECT, 2, (u32) &stage1MituyaPlacesEne0FAParam },
    { 0, -12, 5, 68, ENE_CONNECT, 2, (u32) &stage1MituyaPlacesEne0FAParam },
    { 0, 0, 0, 52, ENE_DOORS_SET_47, 2, (u32) &stage1MituyaPlacesEne127Param },
    { 0, 0, -1, 10, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, -50, 17, -6, ENE_ROOM_YADOYA_STEAM, 0, 0 },
    { 0, -32, 0, -20, ENE_SHOP_CHA_BABA, 2, (u32) &stage1MituyaPlacesEne19FParam },
    { 0, 18, 0, -19, ENE_CHOUNIN, 2, (u32) &stage1MituyaPlacesEne138Param },
    { 0, 15, 0, 35, ENE_SHOP_MITUYA_OMITU, 2, (u32) &stage1MituyaPlacesEne130Param },
};

void stage1MituyaInit(void) {
    stageBuildFixedCam(0x28, 0, 0.0f, 0.0f, 0.0f, (u32) stage1MituyaPlaces, 9, 85.0f, 20.0f, -35.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(5.0f, 2.0f, 0.376991123f);
}
