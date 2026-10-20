#include "tlb.h"

extern ExitData kirikabu1PlacesAEne0F8Param[];
extern ConnectData kirikabu1PlacesBEne0FAParam0[];
extern ConnectData kirikabu1PlacesBEne0FAParam1[];
extern ConnectData kirikabu2PlacesBEne0FAParam0[];
extern ConnectData kirikabu2PlacesBEne0FAParam1[];
extern ConnectData kirikabu2PlacesBEne0FAParam2[];
extern ConnectData kirikabu2PlacesAEne0FAParam0[];
extern ConnectData kirikabu2PlacesAEne0FAParam1[];
extern ConnectData kirikabu3PlacesBEne0FAParam0[];
extern ConnectData kirikabu3PlacesBEne0FAParam1[];
extern ConnectData kirikabu3PlacesBEne0FAParam2[];
extern ConnectData kirikabu3PlacesAEne0FAParam0[];
extern ConnectData kirikabu3PlacesAEne0FAParam1[];
extern ConnectData kirikabu4PlacesEne0FAParam0[];
extern ConnectData kirikabu4PlacesEne0FAParam1[];
extern ConnectData kirikabu4PlacesEne0FAParam2[];
extern ConnectData kirikabu4PlacesEne0FAParam3[];
extern ConnectData kirikabu4PlacesEne0FAParam4[];
extern ConnectData kirikabu4PlacesEne0FAParam5[];
extern ConnectData kirikabu4PlacesEne0FAParam6[];
extern ConnectData kirikabu4PlacesEne0FAParam7[];
extern ConnectData kirikabu4Places2Ene0FAParam[];
extern ExitData kirikabu5Places2Ene0F8Param[];
extern ConnectData kirikabu5PlacesEne0FAParam0[];
extern ConnectData kirikabu5PlacesEne0FAParam2[];
extern ConnectData kirikabu5PlacesEne0FAParam1[];
extern ConnectData kirikabu5Places2Ene0FAParam[];
extern PersonData kirikabu1PlacesSwEne1E4Param4[];
extern PersonData kirikabu1PlacesSwEne1E4Param5[];
extern u16 kirikabu1PlacesAEne13DParamPath[];
extern PersonData kirikabu1PlacesAEne13DParam[];
extern u16 kirikabu1PlacesAEne140ParamPath[];
extern PersonData kirikabu1PlacesAEne140Param[];
extern u16 kirikabu1PlacesAEne13CParamPath[];
extern PersonData kirikabu1PlacesAEne13CParam[];
extern PersonData kirikabu2PlacesSwEne1E4Param4[];
extern PersonData kirikabu2PlacesSwEne1E4Param5[];
extern u16 kirikabu2PlacesAEne139ParamPath[];
extern PersonData kirikabu2PlacesAEne139Param[];
extern u16 kirikabu2PlacesAEne140ParamPath[];
extern PersonData kirikabu2PlacesAEne140Param[];
extern u16 kirikabu2PlacesAEne138ParamPath[];
extern PersonData kirikabu2PlacesAEne138Param[];
extern u16 kirikabu2PlacesAEne13EParamPath[];
extern PersonData kirikabu2PlacesAEne13EParam[];
extern u16 kirikabu2PlacesAEne13FParamPath[];
extern PersonData kirikabu2PlacesAEne13FParam[];
extern PersonData kirikabu3PlacesSwEne1E4Param7[];
extern PersonData kirikabu3PlacesSwEne1E4Param8[];
extern DoorsData kirikabu3PlacesBEne19BParam[];
extern u16 kirikabu3PlacesAEne138ParamPath[];
extern PersonData kirikabu3PlacesAEne138Param[];
extern u16 kirikabu3PlacesAEne13EParam7Path[];
extern PersonData kirikabu3PlacesAEne13EParam7[];
extern u16 kirikabu3PlacesAEne13EParam8Path[];
extern PersonData kirikabu3PlacesAEne13EParam8[];
extern u16 kirikabu3PlacesAEne13AParamPath[];
extern PersonData kirikabu3PlacesAEne13AParam[];
extern u16 kirikabu3PlacesAEne13CParamPath[];
extern PersonData kirikabu3PlacesAEne13CParam[];
extern u16 kirikabu3PlacesAEne13BParamPath[];
extern PersonData kirikabu3PlacesAEne13BParam[];
extern u32 kirikabu4UnusedPersonPath[];
extern PersonData kirikabu4PlacesSwEne1E4Param[];
extern DoorsData kirikabu4PlacesEne19BParam[];
extern u16 kirikabu4Places2Ene138Param1Path[];
extern PersonData kirikabu4Places2Ene138Param1[];
extern u16 kirikabu4Places2Ene140ParamPath[];
extern PersonData kirikabu4Places2Ene140Param[];
extern u16 kirikabu4Places2Ene13FParamPath[];
extern PersonData kirikabu4Places2Ene13FParam[];
extern u16 kirikabu4Places2Ene13AParam4Path[];
extern PersonData kirikabu4Places2Ene13AParam4[];
extern u16 kirikabu4Places2Ene13AParam5Path[];
extern PersonData kirikabu4Places2Ene13AParam5[];
extern u16 kirikabu4Places2Ene13CParamPath[];
extern PersonData kirikabu4Places2Ene13CParam[];
extern u16 kirikabu4Places2Ene13BParamPath[];
extern PersonData kirikabu4Places2Ene13BParam[];
extern u16 kirikabu4Places2Ene13EParamPath[];
extern PersonData kirikabu4Places2Ene13EParam[];
extern u16 kirikabu4Places2Ene138Param9Path[];
extern PersonData kirikabu4Places2Ene138Param9[];
extern PersonData kirikabu5PlacesSwEne1E4Param8[];
extern PersonData kirikabu5PlacesSwEne1E4Param9[];
extern u16 kirikabu5Places2Ene13DParamPath[];
extern PersonData kirikabu5Places2Ene13DParam[];
extern u16 kirikabu5Places2Ene140ParamPath[];
extern PersonData kirikabu5Places2Ene140Param[];
extern u16 kirikabu5Places2Ene13CParamPath[];
extern PersonData kirikabu5Places2Ene13CParam[];
extern u16 kirikabu5Places2Ene13BParamPath[];
extern PersonData kirikabu5Places2Ene13BParam[];
extern u16 kirikabu5UnusedPersonPath[];
extern PersonData kirikabu5Places2Ene13EParam[];
ConnectData kirikabu1UnusedEne0FAParam[] = {
    { 15.0f, 200, 0, -15.0f, 0.0f, 78.0f, 3.1415927f, 0x0000, 0, 0 },
};
ExitData kirikabu1PlacesAEne0F8Param[] = {
    { 25.0f, 3, 3, -1, 0 },
};
ConnectData kirikabu1PlacesBEne0FAParam0[] = {
    { 20.0f, 96, 0, -275.0f, 0.0f, 54.0f, 1.5707964f, 0x0000, 0, 0 },
};
ConnectData kirikabu1PlacesBEne0FAParam1[] = {
    { 15.0f, 230, 0, 31.0f, 0.0f, 80.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData kirikabu2PlacesBEne0FAParam0[] = {
    { 20.0f, 95, 0, 105.0f, 0.0f, 20.0f, -1.5707964f, 0x0000, 0, 0 },
};
ConnectData kirikabu2PlacesBEne0FAParam1[] = {
    { 20.0f, 97, 0, -154.0f, -3.0f, 156.0f, 1.5707964f, 0x0000, 0, 0 },
};
ConnectData kirikabu2PlacesBEne0FAParam2[] = {
    { 20.0f, 111, 0, 325.0f, 10.0f, 197.0f, 4.712389f, 0x0000, 0, 0 },
};
ConnectData kirikabu2PlacesAEne0FAParam0[] = {
    { 15.0f, 231, 0, -15.0f, 0.0f, 78.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData kirikabu2PlacesAEne0FAParam1[] = {
    { 15.0f, 232, 0, 38.0f, 0.0f, 70.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData kirikabu3PlacesBEne0FAParam0[] = {
    { 20.0f, 96, 0, -260.0f, 0.0f, 21.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData kirikabu3PlacesBEne0FAParam1[] = {
    { 20.0f, 111, 0, -276.0f, 252.0f, 175.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData kirikabu3PlacesBEne0FAParam2[] = {
    { 20.0f, 112, 0, 127.0f, 10.0f, -2.0f, 1.5707964f, 0x0000, 0, 0 },
};
ConnectData kirikabu3PlacesAEne0FAParam0[] = {
    { 10.0f, 233, 0, 58.0f, 0.0f, 54.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData kirikabu3PlacesAEne0FAParam1[] = {
    { 15.0f, 234, 0, -15.0f, 0.0f, 78.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData kirikabu4PlacesEne0FAParam0[] = {
    { 20.0f, 96, 0, 259.0f, 0.0f, 2.0f, 4.712389f, 0x0000, 0, 0 },
};
ConnectData kirikabu4PlacesEne0FAParam1[] = {
    { 20.0f, 97, 0, 251.0f, -2.0f, -68.0f, -0.62831855f, 0x0000, 0, 0 },
};
ConnectData kirikabu4PlacesEne0FAParam2[] = {
    { 15.0f, 237, 0, 5.0f, 0.0f, 70.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData kirikabu4PlacesEne0FAParam3[] = {
    { 15.0f, 238, 0, 5.0f, 0.0f, 70.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData kirikabu4PlacesEne0FAParam4[] = {
    { 15.0f, 239, 0, 5.0f, 0.0f, 70.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData kirikabu4PlacesEne0FAParam5[] = {
    { 15.0f, 240, 0, 5.0f, 0.0f, 70.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData kirikabu4PlacesEne0FAParam6[] = {
    { 15.0f, 241, 0, 5.0f, 0.0f, 70.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData kirikabu4PlacesEne0FAParam7[] = {
    { 15.0f, 242, 0, 5.0f, 0.0f, 70.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData kirikabu4Places2Ene0FAParam[] = {
    { 10.0f, 235, 0, 58.0f, 0.0f, 54.0f, 3.1415927f, 0x0000, 0, 0 },
};
ExitData kirikabu5Places2Ene0F8Param[] = {
    { 25.0f, 3, 3, -1, 0 },
};
ConnectData kirikabu5PlacesEne0FAParam0[] = {
    { 20.0f, 97, 0, -159.0f, 0.0f, 66.0f, 2.1991148f, 0x0000, 0, 0 },
};
ConnectData kirikabu5PlacesEne0FAParam2[] = {
    { 10.0f, 112, 0, -117.0f, 10.0f, 1.0f, -1.5707964f, 0x0160, 0, 0 },
};
ConnectData kirikabu5PlacesEne0FAParam1[] = {
    { 10.0f, 112, 0, -4.0f, 100.0f, -60.0f, 0.0f, 0x0160, 0, 0 },
};
ConnectData kirikabu5Places2Ene0FAParam[] = {
    { 15.0f, 236, 0, -40.0f, 0.0f, 110.0f, 3.1415927f, 0x0007, 0, 0 },
};
PersonData kirikabu1PlacesSwEne1E4Param4[] = {
    { 0, 0x00000000, { 20, 50 }, 2, 0.61359f, 0 },
};
PersonData kirikabu1PlacesSwEne1E4Param5[] = {
    { 1, 0x00000000, { 15, 50 }, 2, 0.0f, 0 },
};
u16 kirikabu1PlacesAEne13DParamPath[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData kirikabu1PlacesAEne13DParam[] = {
    { 622, (u32) &kirikabu1PlacesAEne13DParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
u16 kirikabu1PlacesAEne140ParamPath[] = { 0x0002, 0x003C, 0x0004, 0x003C, 0x0002, 0x012C, 0x0004, 0x003C, 0x0001, 0x0000 };
PersonData kirikabu1PlacesAEne140Param[] = {
    { 623, (u32) &kirikabu1PlacesAEne140ParamPath, { 5, 8 }, 0, 0.0f, 0 },
};
u16 kirikabu1PlacesAEne13CParamPath[] = { 0x0004, 0x0014, 0x0000, 0x007C, 0x001A, 0x0001, 0x0004, 0x0014, 0x0000, 0x0048, 0x0019, 0x0001, 0x0001, 0x0000 };
PersonData kirikabu1PlacesAEne13CParam[] = {
    { 624, (u32) &kirikabu1PlacesAEne13CParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef kirikabu1PlacesA[] = {
    { 0, -148, 0, -4, ENE_EXIT, 2, (u32) &kirikabu1PlacesAEne0F8Param },
    { 0, -76, 2, -15, ENE_DERO_KARASU_YAKUNIN, 2, (u32) &kirikabu1PlacesAEne13DParam },
    { 0, 24, 2, 3, ENE_MUSUME, 2, (u32) &kirikabu1PlacesAEne140Param },
    { 0, 82, 2, 26, ENE_ZISAN, 2, (u32) &kirikabu1PlacesAEne13CParam },
};
EneDef kirikabu1PlacesB[] = {
    { 0, 166, 0, 16, ENE_CONNECT, 2, (u32) &kirikabu1PlacesBEne0FAParam0 },
    { 0, 75, 0, -34, ENE_CONNECT, 2, (u32) &kirikabu1PlacesBEne0FAParam1 },
    { 0, 0, -10, 0, ENE_KAWA_KIRIKAWA1, 0, 0 },
    { 0, 116, 10, -5, ENE_INDEX, 2, 4 },
};
EneDef kirikabu1PlacesSw[] = {
    { 0, 0, 0, 0, ENE_DEMO_KAKURENBO, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, -51, 2, -25, ENE_BLUE, 2, 1 },
    { 0, -51, 2, -14, ENE_KANBAN, 2, 1 },
    { 0, -119, 0, -5, ENE_KAKURENBO_JIZOU, 2, (u32) &kirikabu1PlacesSwEne1E4Param4 },
    { 0, 74, 0, -14, ENE_KAKURENBO_JIZOU, 2, (u32) &kirikabu1PlacesSwEne1E4Param5 },
};
PersonData kirikabu2PlacesSwEne1E4Param4[] = {
    { 1, 0x00000000, { 15, 50 }, 2, 0.220891f, 0 },
};
PersonData kirikabu2PlacesSwEne1E4Param5[] = {
    { 1, 0x00000000, { 15, 50 }, 2, 0.0f, 0 },
};
u16 kirikabu2PlacesAEne139ParamPath[] = { 0x0000, 0xFF73, 0x0020, 0x0004, 0x0000, 0xFF33, 0x0030, 0x0004, 0x0000, 0xFF00, 0x002F, 0x0004, 0x0000, 0x00DD, 0x002E, 0x0004, 0x0000, 0x0101, 0xFFFE, 0x0004, 0x0000, 0x0141, 0x0004, 0x0004, 0x0004, 0x00C8, 0x0000, 0x00F3, 0x000D, 0x0004, 0x0000, 0x00C7, 0x002D, 0x0004, 0x0000, 0x00F3, 0x000D, 0x0004, 0x0001, 0x0000 };
PersonData kirikabu2PlacesAEne139Param[] = {
    { 625, (u32) &kirikabu2PlacesAEne139ParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
u16 kirikabu2PlacesAEne140ParamPath[] = { 0x0000, 0xFFE5, 0xFFE1, 0x0002, 0x0004, 0x001E, 0x0000, 0xFFB8, 0x0005, 0x0002, 0x0004, 0x003C, 0x0000, 0x0000, 0x0000, 0x0002, 0x0004, 0x001E, 0x0001, 0x0000 };
PersonData kirikabu2PlacesAEne140Param[] = {
    { 626, (u32) &kirikabu2PlacesAEne140ParamPath, { 5, 8 }, 0, 0.0f, 0 },
};
u16 kirikabu2PlacesAEne138ParamPath[] = { 0x0000, 0x0032, 0x0020, 0x0002, 0x0004, 0x0014, 0x0000, 0x0050, 0xFFD8, 0x0002, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData kirikabu2PlacesAEne138Param[] = {
    { 627, (u32) &kirikabu2PlacesAEne138ParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
u16 kirikabu2PlacesAEne13EParamPath[] = { 0x0000, 0x0064, 0xFFEE, 0x0002, 0x0000, 0x008A, 0x0018, 0x0002, 0x0000, 0x00B3, 0x0014, 0x0002, 0x0004, 0x003C, 0x0000, 0x0089, 0x0014, 0x0002, 0x0001, 0x0000 };
PersonData kirikabu2PlacesAEne13EParam[] = {
    { 628, (u32) &kirikabu2PlacesAEne13EParamPath, { 5, 8 }, 0, 0.0f, 0 },
};
u16 kirikabu2PlacesAEne13FParamPath[] = { 0x0002, 0x003C, 0x0004, 0x003C, 0x0002, 0x012C, 0x0004, 0x003C, 0x0001, 0x0000, 0x0000, 0x00C5, 0x0014, 0x0002, 0x0004, 0x003C, 0x0000, 0x0104, 0x0028, 0x0002, 0x0004, 0x003C, 0x0001, 0x0000 };
PersonData kirikabu2PlacesAEne13FParam[] = {
    { 629, (u32) &kirikabu2PlacesAEne13FParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef kirikabu2PlacesA[] = {
    { 0, -199, 3, -53, ENE_CONNECT, 2, (u32) &kirikabu2PlacesAEne0FAParam0 },
    { 0, 223, 2, -29, ENE_CONNECT, 2, (u32) &kirikabu2PlacesAEne0FAParam1 },
    { 0, -116, 2, 24, ENE_HIKYAKU, 2, (u32) &kirikabu2PlacesAEne139Param },
    { 0, -27, 2, -31, ENE_MUSUME, 2, (u32) &kirikabu2PlacesAEne140Param },
    { 0, 50, 2, 32, ENE_CHOUNIN, 2, (u32) &kirikabu2PlacesAEne138Param },
    { 0, 101, 2, -18, ENE_KODOMO, 2, (u32) &kirikabu2PlacesAEne13EParam },
    { 0, 197, 2, 20, ENE_INU, 2, (u32) &kirikabu2PlacesAEne13FParam },
    { 0, -90, 1, 8, ENE_OKE_BODY, 2, 6 },
};
EneDef kirikabu2PlacesB[] = {
    { 0, -312, 0, 40, ENE_CONNECT, 2, (u32) &kirikabu2PlacesBEne0FAParam0 },
    { 0, -262, 10, -2, ENE_CONNECT, 2, (u32) &kirikabu2PlacesBEne0FAParam1 },
    { 0, 293, 10, 6, ENE_CONNECT, 2, (u32) &kirikabu2PlacesBEne0FAParam2 },
    { 0, 12, -17, -8, ENE_KAWA_KIRIKAWA2, 0, 0 },
    { 0, 65, 52, -220, ENE_KAWA_KIRIKAWA2_SMALL, 0, 0 },
    { 0, -293, 10, 25, ENE_INDEX, 2, 0 },
    { 0, -273, 10, 21, ENE_INDEX, 2, 2 },
    { 0, 251, 10, -4, ENE_INDEX, 2, 4 },
};
EneDef kirikabu2PlacesSw[] = {
    { 0, 0, 0, 0, ENE_DEMO_KAKURENBO, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, -90, 1, 8, ENE_YELLOW, 2, 6 },
    { 0, -90, 1, 8, ENE_OKE_BODY, 2, 6 },
    { 0, -200, 2, -17, ENE_KAKURENBO_JIZOU, 2, (u32) &kirikabu2PlacesSwEne1E4Param4 },
    { 0, 224, 2, 2, ENE_KAKURENBO_JIZOU, 2, (u32) &kirikabu2PlacesSwEne1E4Param5 },
};
PersonData kirikabu3PlacesSwEne1E4Param7[] = {
    { 1, 0x00000000, { 15, 50 }, 2, 0.0f, 0 },
};
PersonData kirikabu3PlacesSwEne1E4Param8[] = {
    { 1, 0x00000000, { 15, 50 }, 2, 0.0f, 0 },
};
DoorsData kirikabu3PlacesBEne19BParam[] = {
    { 0.0f, 0 },
};
u16 kirikabu3PlacesAEne138ParamPath[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData kirikabu3PlacesAEne138Param[] = {
    { 630, (u32) &kirikabu3PlacesAEne138ParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
u16 kirikabu3PlacesAEne13EParam7Path[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData kirikabu3PlacesAEne13EParam7[] = {
    { 631, (u32) &kirikabu3PlacesAEne13EParam7Path, { 5, 8 }, 0, 0.0f, 0 },
};
u16 kirikabu3PlacesAEne13EParam8Path[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData kirikabu3PlacesAEne13EParam8[] = {
    { 632, (u32) &kirikabu3PlacesAEne13EParam8Path, { 5, 8 }, 0, 0.0f, 0 },
};
u16 kirikabu3PlacesAEne13AParamPath[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData kirikabu3PlacesAEne13AParam[] = {
    { 633, (u32) &kirikabu3PlacesAEne13AParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
u16 kirikabu3PlacesAEne13CParamPath[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData kirikabu3PlacesAEne13CParam[] = {
    { 634, (u32) &kirikabu3PlacesAEne13CParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
u16 kirikabu3PlacesAEne13BParamPath[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData kirikabu3PlacesAEne13BParam[] = {
    { 635, (u32) &kirikabu3PlacesAEne13BParamPath, { 5, 8 }, 1, 0.0f, 0 },
};
EneDef kirikabu3PlacesA[] = {
    { 0, -1, 4, 7, ENE_CONNECT, 2, (u32) &kirikabu3PlacesAEne0FAParam0 },
    { 0, 89, 3, -22, ENE_CONNECT, 2, (u32) &kirikabu3PlacesAEne0FAParam1 },
    { 0, 2, -9, 142, ENE_ITEM_OOIRI_MAIN, 2, 415 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, -117, 2, -9, ENE_CHOUNIN, 2, (u32) &kirikabu3PlacesAEne138Param },
    { 0, -25, 2, 56, ENE_KODOMO, 2, (u32) &kirikabu3PlacesAEne13EParam7 },
    { 0, 29, -8, 129, ENE_KODOMO, 2, (u32) &kirikabu3PlacesAEne13EParam8 },
    { 0, 168, 2, -39, ENE_SHOUNIN, 2, (u32) &kirikabu3PlacesAEne13AParam },
    { 0, 70, 2, 16, ENE_ZISAN, 2, (u32) &kirikabu3PlacesAEne13CParam },
    { 0, 54, -2, 66, ENE_ROUNIN, 2, (u32) &kirikabu3PlacesAEne13BParam },
    { 0, -131, 1, -34, ENE_OKE_BODY, 2, 3 },
    { 0, 112, 2, 18, ENE_KANBAN, 2, 2 },
};
EneDef kirikabu3PlacesB[] = {
    { 0, -186, 6, 181, ENE_CONNECT, 2, (u32) &kirikabu3PlacesBEne0FAParam0 },
    { 0, 280, 13, -79, ENE_CONNECT, 2, (u32) &kirikabu3PlacesBEne0FAParam1 },
    { 0, -200, 0, 64, ENE_CONNECT, 2, (u32) &kirikabu3PlacesBEne0FAParam2 },
    { 0, 0, -12, -255, ENE_KAWA_KIRIKAWA1, 0, 0 },
    { 0, 0, -37, -242, ENE_KAWA_KIRIKAWA1, 0, 0 },
    { 0, -2, 5, 9, ENE_DOORS_SET_96, 2, (u32) &kirikabu3PlacesBEne19BParam },
    { 0, 188, 5, -69, ENE_INDEX, 2, 4 },
    { 0, -151, 10, 51, ENE_INDEX, 2, 0 },
};
EneDef kirikabu3PlacesSw[] = {
    { 0, 0, 0, 0, ENE_DEMO_KAKURENBO, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, 159, 2, 130, ENE_RED, 2, 1 },
    { 0, -131, 1, -34, ENE_PINK, 2, 3 },
    { 0, -131, 1, -34, ENE_OKE_BODY, 2, 3 },
    { 0, 135, 2, -1, ENE_BLUE, 2, 2 },
    { 0, 135, 2, 10, ENE_KANBAN, 2, 2 },
    { 0, 1, 2, 27, ENE_KAKURENBO_JIZOU, 2, (u32) &kirikabu3PlacesSwEne1E4Param7 },
    { 0, 89, 2, -10, ENE_KAKURENBO_JIZOU, 2, (u32) &kirikabu3PlacesSwEne1E4Param8 },
};
u32 kirikabu4UnusedPersonPath[] = { 0x0002012C, 0x00040014, 0x0002001E, 0x00040014, 0x00020000, 0x00040014, 0x00010000, 0x000001F0, (u32) &kirikabu4UnusedPersonPath, 0x05080200, 0x00000000, 0x00000000 };
PersonData kirikabu4PlacesSwEne1E4Param[] = {
    { 1, 0x00000000, { 25, 50 }, 2, 0.0f, 0 },
};
DoorsData kirikabu4PlacesEne19BParam[] = {
    { 0.0f, 0 },
};
u16 kirikabu4Places2Ene138Param1Path[] = { 0x0000, 0xFF15, 0x009B, 0x0001, 0x0000, 0xFF15, 0x0045, 0x0001, 0x0000, 0xFEBE, 0x009F, 0x0001, 0x0000, 0xFF0E, 0x009F, 0x0001, 0x0004, 0x001E, 0x0001, 0x0000 };
PersonData kirikabu4Places2Ene138Param1[] = {
    { 636, (u32) &kirikabu4Places2Ene138Param1Path, { 5, 8 }, 1, 0.0f, 0 },
};
u16 kirikabu4Places2Ene140ParamPath[] = { 0x0000, 0xFEC7, 0x007F, 0x0001, 0x0004, 0x0014, 0x0000, 0xFEC7, 0x00A1, 0x0001, 0x0004, 0x0014, 0x0000, 0xFEB4, 0x009F, 0x0001, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData kirikabu4Places2Ene140Param[] = {
    { 637, (u32) &kirikabu4Places2Ene140ParamPath, { 5, 8 }, 0, 0.0f, 0 },
};
u16 kirikabu4Places2Ene13FParamPath[] = { 0x0000, 0x0005, 0x0050, 0x0001, 0x0004, 0x001E, 0x0000, 0x0005, 0x0010, 0x0001, 0x0004, 0x001E, 0x0001, 0x0000 };
PersonData kirikabu4Places2Ene13FParam[] = {
    { 638, (u32) &kirikabu4Places2Ene13FParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
u16 kirikabu4Places2Ene13AParam4Path[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData kirikabu4Places2Ene13AParam4[] = {
    { 639, (u32) &kirikabu4Places2Ene13AParam4Path, { 5, 8 }, 2, 0.0f, 0 },
};
u16 kirikabu4Places2Ene13AParam5Path[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData kirikabu4Places2Ene13AParam5[] = {
    { 640, (u32) &kirikabu4Places2Ene13AParam5Path, { 5, 8 }, 2, 0.0f, 0 },
};
u16 kirikabu4Places2Ene13CParamPath[] = { 0x0000, 0x0036, 0x0017, 0x0001, 0x0004, 0x0078, 0x0000, 0x001D, 0x003E, 0x0001, 0x0004, 0x003C, 0x0001, 0x0000 };
PersonData kirikabu4Places2Ene13CParam[] = {
    { 641, (u32) &kirikabu4Places2Ene13CParamPath, { 5, 8 }, 0, 0.0f, 0 },
};
u16 kirikabu4Places2Ene13BParamPath[] = { 0x0000, 0x0092, 0x007B, 0x0001, 0x0004, 0x0014, 0x0000, 0x0077, 0x007D, 0x0001, 0x0004, 0x003C, 0x0000, 0x007A, 0x0090, 0x0001, 0x0000, 0x006A, 0x0092, 0x0001, 0x0000, 0x006A, 0x008F, 0x0001, 0x0004, 0x003C, 0x0000, 0x006A, 0x0094, 0x0001, 0x0000, 0x00AB, 0x0094, 0x0001, 0x0001, 0x0000 };
PersonData kirikabu4Places2Ene13BParam[] = {
    { 642, (u32) &kirikabu4Places2Ene13BParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
u16 kirikabu4Places2Ene13EParamPath[] = { 0x0004, 0x001E, 0x0004, 0x001E, 0x0001, 0x0000 };
PersonData kirikabu4Places2Ene13EParam[] = {
    { 643, (u32) &kirikabu4Places2Ene13EParamPath, { 5, 8 }, 0, 0.0f, 0 },
};
u16 kirikabu4Places2Ene138Param9Path[] = { 0x0000, 0x0063, 0x0082, 0x0001, 0x0000, 0x008C, 0x0082, 0x0001, 0x0000, 0x008C, 0x005F, 0x0001, 0x0000, 0x00AD, 0x005F, 0x0001, 0x0004, 0x0028, 0x0000, 0x008C, 0x005F, 0x0001, 0x0000, 0x008C, 0x0082, 0x0001, 0x0000, 0x0070, 0x00C1, 0x0001, 0x0001, 0x0000 };
PersonData kirikabu4Places2Ene138Param9[] = {
    { 644, (u32) &kirikabu4Places2Ene138Param9Path, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef kirikabu4Places2[] = {
    { 0, -244, 170, 155, ENE_CONNECT, 2, (u32) &kirikabu4Places2Ene0FAParam },
    { 0, -235, 402, 155, ENE_CHOUNIN, 2, (u32) &kirikabu4Places2Ene138Param1 },
    { 0, -313, 402, 127, ENE_MUSUME, 2, (u32) &kirikabu4Places2Ene140Param },
    { 0, 5, 417, 80, ENE_INU, 2, (u32) &kirikabu4Places2Ene13FParam },
    { 0, 371, 204, 166, ENE_SHOUNIN, 2, (u32) &kirikabu4Places2Ene13AParam4 },
    { 0, -232, 252, 177, ENE_SHOUNIN, 2, (u32) &kirikabu4Places2Ene13AParam5 },
    { 0, 54, 417, 23, ENE_ZISAN, 2, (u32) &kirikabu4Places2Ene13CParam },
    { 0, 146, 306, 123, ENE_ROUNIN, 2, (u32) &kirikabu4Places2Ene13BParam },
    { 0, 277, 202, 200, ENE_KODOMO, 2, (u32) &kirikabu4Places2Ene13EParam },
    { 0, 99, 149, 130, ENE_CHOUNIN, 2, (u32) &kirikabu4Places2Ene138Param9 },
    { 0, -271, 6, 233, ENE_OKE_BODY, 2, 2 },
    { 0, -273, 253, 169, ENE_OKE_BODY, 2, 1 },
    { 0, 47, 416, 73, ENE_OKE_BODY, 2, 4 },
    { 0, -274, 401, 141, ENE_OKE_BODY, 2, 5 },
    { 0, 105, 306, 131, ENE_KANBAN, 2, 3 },
};
EneDef kirikabu4Places[] = {
    { 0, 372, 12, 195, ENE_CONNECT, 2, (u32) &kirikabu4PlacesEne0FAParam0 },
    { 0, -278, 255, 103, ENE_CONNECT, 2, (u32) &kirikabu4PlacesEne0FAParam1 },
    { 0, 150, 22, 52, ENE_CONNECT, 2, (u32) &kirikabu4PlacesEne0FAParam2 },
    { 0, 30, 17, 20, ENE_CONNECT, 2, (u32) &kirikabu4PlacesEne0FAParam3 },
    { 0, -326, 0, 67, ENE_CONNECT, 2, (u32) &kirikabu4PlacesEne0FAParam4 },
    { 0, 77, 305, 35, ENE_CONNECT, 2, (u32) &kirikabu4PlacesEne0FAParam5 },
    { 0, 27, 297, 62, ENE_CONNECT, 2, (u32) &kirikabu4PlacesEne0FAParam6 },
    { 0, -229, 256, 100, ENE_CONNECT, 2, (u32) &kirikabu4PlacesEne0FAParam7 },
    { 0, 0, -5, 0, ENE_KAWA_KIRIKAWA1, 0, 0 },
    { 0, -127, 325, -37, ENE_KAWA_KIRIKAWA2_SMALL, 0, 0 },
    { 0, -244, 171, 173, ENE_DOORS_SET_96, 2, (u32) &kirikabu4PlacesEne19BParam },
    { 0, 297, 20, 182, ENE_INDEX, 2, 4 },
    { 0, -293, 260, 145, ENE_INDEX, 2, 2 },
};
EneDef kirikabu4PlacesSw[] = {
    { 0, 0, 0, 0, ENE_DEMO_KAKURENBO, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, 167, 148, 122, ENE_RED, 2, 2 },
    { 0, -271, 6, 233, ENE_PINK, 2, 2 },
    { 0, -271, 6, 233, ENE_OKE_BODY, 2, 2 },
    { 0, -273, 253, 169, ENE_PINK, 2, 1 },
    { 0, -273, 253, 169, ENE_OKE_BODY, 2, 1 },
    { 0, 47, 416, 73, ENE_YELLOW, 2, 4 },
    { 0, 47, 416, 73, ENE_OKE_BODY, 2, 4 },
    { 0, -274, 401, 141, ENE_YELLOW, 2, 5 },
    { 0, -274, 401, 141, ENE_OKE_BODY, 2, 5 },
    { 0, 105, 306, 120, ENE_BLUE, 2, 3 },
    { 0, 105, 306, 131, ENE_KANBAN, 2, 3 },
    { 0, -245, 169, 177, ENE_KAKURENBO_JIZOU, 2, (u32) &kirikabu4PlacesSwEne1E4Param },
};
PersonData kirikabu5PlacesSwEne1E4Param8[] = {
    { 0, 0x00000000, { 20, 50 }, 2, 0.14726f, 0 },
};
PersonData kirikabu5PlacesSwEne1E4Param9[] = {
    { 1, 0x00000000, { 15, 50 }, 2, 3.307254f, 0 },
};
u16 kirikabu5Places2Ene13DParamPath[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData kirikabu5Places2Ene13DParam[] = {
    { 645, (u32) &kirikabu5Places2Ene13DParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
u16 kirikabu5Places2Ene140ParamPath[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData kirikabu5Places2Ene140Param[] = {
    { 646, (u32) &kirikabu5Places2Ene140ParamPath, { 5, 8 }, 0, 0.0f, 0 },
};
u16 kirikabu5Places2Ene13CParamPath[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData kirikabu5Places2Ene13CParam[] = {
    { 647, (u32) &kirikabu5Places2Ene13CParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
u16 kirikabu5Places2Ene13BParamPath[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000, 0x0001, 0x0000 };
PersonData kirikabu5Places2Ene13BParam[] = {
    { 648, (u32) &kirikabu5Places2Ene13BParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
u16 kirikabu5UnusedPersonPath[] = { 0x0002, 0x003C, 0x0004, 0x0014, 0x0002, 0x012C, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData kirikabu5Places2Ene13EParam[] = {
    { 649, (u32) &kirikabu5Places2Ene13BParamPath, { 5, 8 }, 0, 0.0f, 0 },
};
EneDef kirikabu5Places2[] = {
    { 0, -2, 10, 90, ENE_EXIT, 2, (u32) &kirikabu5Places2Ene0F8Param },
    { 0, 0, 10, -89, ENE_CONNECT, 2, (u32) &kirikabu5Places2Ene0FAParam },
    { 0, -84, 102, 34, ENE_DERO_KARASU_YAKUNIN, 2, (u32) &kirikabu5Places2Ene13DParam },
    { 0, -158, 4, 22, ENE_MUSUME, 2, (u32) &kirikabu5Places2Ene140Param },
    { 0, 142, 4, -33, ENE_ZISAN, 2, (u32) &kirikabu5Places2Ene13CParam },
    { 0, -106, 4, 98, ENE_ROUNIN, 2, (u32) &kirikabu5Places2Ene13BParam },
    { 0, 24, 102, 50, ENE_KODOMO, 2, (u32) &kirikabu5Places2Ene13EParam },
};
EneDef kirikabu5Places[] = {
    { 0, 88, 10, 1, ENE_CONNECT, 2, (u32) &kirikabu5PlacesEne0FAParam0 },
    { 0, -89, 20, 0, ENE_CONNECT, 2, (u32) &kirikabu5PlacesEne0FAParam1 },
    { 0, -4, 108, -89, ENE_CONNECT, 2, (u32) &kirikabu5PlacesEne0FAParam2 },
    { 0, 0, -91, 0, ENE_KAWA_KIRIKAWA5, 0, 0 },
    { 0, 131, 12, 19, ENE_INDEX, 2, 16777218 },
};
EneDef kirikabu5PlacesSw[] = {
    { 0, 0, 0, 0, ENE_DEMO_KAKURENBO, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, 0, 0, 0, ENE_KODOMO_RUNRUN, 0, 0 },
    { 0, 0, 0, 0, ENE_KODOMO_RUNRUN_BLUE, 0, 0 },
    { 0, 0, 0, 0, ENE_KODOMO_RUNRUN_YELLOW, 0, 0 },
    { 0, 0, 0, 0, ENE_KODOMO_RUNRUN_PINK, 0, 0 },
    { 0, 0, 0, 0, ENE_KODOMO_RUNRUN_GREEN, 0, 0 },
    { 0, 0, 0, 0, ENE_KODOMO_RUNRUN_BLACK, 0, 0 },
    { 0, 0, 12, 112, ENE_KAKURENBO_JIZOU, 2, (u32) &kirikabu5PlacesSwEne1E4Param8 },
    { 0, -2, 12, -122, ENE_KAKURENBO_JIZOU, 2, (u32) &kirikabu5PlacesSwEne1E4Param9 },
};

void kirikabu1Init(void) {
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        stageBuildZoneCam2(0x58, 0, -105.0f, 0.0f, -5.0f, (u32) kirikabu1PlacesB, 4, (u32) kirikabu1PlacesSw, 6);
    } else {
        stageBuildZoneCam2(0x58, 0, -105.0f, 0.0f, -5.0f, (u32) kirikabu1PlacesA, 4, (u32) kirikabu1PlacesB, 4);
    }
    if (!swWorkTest(0x13) || swWorkTest(0xAC)) {
        camSunSetTunes4(0x89, 0x86, 0x87, 0x88);
        camSunSetDawnSound(0x42F);
    }
    sndPlay(0x521);
    camAiSetFar(600);
    camSunSetProgram(6);
    camAiAddLimit(1, -1, 100);
    camAiAddLimit(1, 1, 0x134);
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        eneMakeEx(NULL, ENE_DEMO_KAKURENBO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
    }
}

void kirikabu2Init(void) {
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        stageBuildZoneCam2(0x59, 0, -300.0f, 0.0f, 42.0f, (u32) kirikabu2PlacesB, 8, (u32) kirikabu2PlacesSw, 6);
    } else {
        stageBuildZoneCam2(0x59, 0, -300.0f, 0.0f, 42.0f, (u32) kirikabu2PlacesA, 8, (u32) kirikabu2PlacesB, 8);
    }
    if (!swWorkTest(0x13) || swWorkTest(0xAC)) {
        camSunSetTunes4(0x89, 0x86, 0x87, 0x88);
        camSunSetDawnSound(0x42F);
    }
    sndPlay(0x521);
    camAiSetFar(600);
    camSunSetProgram(6);
    camAiAddLimit(1, -1, 0xB4);
    camAiAddLimit(1, 1, 0x26C);
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        eneMakeEx(NULL, ENE_DEMO_KAKURENBO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
    }
}

void kirikabu3Init(void) {
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        stageBuildZoneCam2(0x5A, 0, -145.0f, 0.0f, 70.0f, (u32) kirikabu3PlacesB, 8, (u32) kirikabu3PlacesSw, 9);
    } else {
        stageBuildZoneCam2(0x5A, 0, -145.0f, 0.0f, 70.0f, (u32) kirikabu3PlacesA, 0xE, (u32) kirikabu3PlacesB, 8);
    }
    if (!swWorkTest(0x13) || swWorkTest(0xAC)) {
        camSunSetTunes4(0x89, 0x86, 0x87, 0x88);
        camSunSetDawnSound(0x42F);
    }
    sndPlay(0x521);
    camAiSetFar(600);
    camSunSetProgram(6);
    camAiAddLimit(1, -1, 0x10E);
    camAiAddLimit(1, 1, 0x262);
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        eneMakeEx(NULL, ENE_DEMO_KAKURENBO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
    }
}

void kirikabu4Init(void) {
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        stageBuildZoneCam2(0x5B, 0, 350.0f, 5.0f, 185.0f, (u32) kirikabu4Places, 0xD, (u32) kirikabu4PlacesSw, 0xE);
    } else {
        stageBuildZoneCam2(0x5B, 0, 350.0f, 5.0f, 185.0f, (u32) kirikabu4Places, 0xD, (u32) kirikabu4Places2, 0xF);
    }
    if (!swWorkTest(0x13) || swWorkTest(0xAC)) {
        camSunSetTunes4(0x89, 0x86, 0x87, 0x88);
        camSunSetDawnSound(0x42F);
    }
    sndPlay(0x521);
    camAiSetFar(600);
    camSunSetProgram(6);
    camAiAddLimit(1, -1, 0x2EE);
    camAiAddLimit(1, 1, 0x578);
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        eneMakeEx(NULL, ENE_DEMO_KAKURENBO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
    }
}

void kirikabu5Init(void) {
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        stageBuildZoneCam2(0x5C, 0, 40.0f, 13.0f, 290.0f, (u32) kirikabu5Places, 5, (u32) kirikabu5PlacesSw, 0xA);
    } else {
        stageBuildZoneCam2(0x5C, 0, 40.0f, 13.0f, 290.0f, (u32) kirikabu5Places, 5, (u32) kirikabu5Places2, 7);
    }
    if (!swWorkTest(0x13) || swWorkTest(0xAC)) {
        camSunSetTunes4(0x89, 0x86, 0x87, 0x88);
        camSunSetDawnSound(0x42F);
    }
    sndPlay(0x521);
    camAiSetFar(600);
    camSunSetProgram(6);
    if (swWorkTest(0x13) && !swWorkTest(0xAC)) {
        eneMakeEx(NULL, ENE_DEMO_KAKURENBO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
    }
}
