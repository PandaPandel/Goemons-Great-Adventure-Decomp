#include "tlb.h"

extern u16 stg0198PlacesEne13CParamPath[];
extern PersonData stg0198PlacesEne13CParam[];
extern u16 stg0198PlacesEne13BParamPath[];
extern PersonData stg0198PlacesEne13BParam[];
extern u16 stg0198PlacesEne138Param21Path[];
extern PersonData stg0198PlacesEne138Param21[];
extern u16 stg0198PlacesEne139ParamPath[];
extern PersonData stg0198PlacesEne139Param[];
extern u16 stg0198PlacesEne13AParamPath[];
extern PersonData stg0198PlacesEne13AParam[];
extern u16 stg0198PlacesEne13EParamPath[];
extern PersonData stg0198PlacesEne13EParam[];
extern u16 stg0198PlacesEne138Param25Path[];
extern PersonData stg0198PlacesEne138Param25[];
extern u16 stg0198PlacesEne13FParamPath[];
extern PersonData stg0198PlacesEne13FParam[];
extern ConnectData stg0198PlacesEne0FAParam0[];
extern ConnectData stg0198PlacesEne0FAParam1[];
extern ConnectData stg0198PlacesEne0FAParam2[];
extern ConnectData stg0198PlacesEne0FAParam3[];
extern ConnectData stg0198PlacesEne0FAParam4[];
extern ConnectData stg0198PlacesEne0FAParam5[];
extern ConnectData stg0198PlacesEne0FAParam6[];
extern ConnectData stg0198PlacesEne0FAParam7[];
extern ConnectData stg0198PlacesEne0FAParam8[];
extern DoorsData stg0198PlacesEne19BParam9[];
extern DoorsData stg0198PlacesEne19BParam10[];
extern DoorsData stg0198PlacesEne19BParam11[];
extern DoorsData stg0198PlacesEne19BParam12[];
extern DoorsData stg0198PlacesEne19BParam13[];
extern DoorsData stg0198PlacesEne19BParam14[];
extern DoorsData stg0198PlacesEne19BParam15[];
extern DoorsData stg0198PlacesEne19BParam16[];
extern ConnectData stg0198PlacesEne0FAParam17[];
void stg0198Build(void);
u16 stg0198PlacesEne13CParamPath[] = { 0x0000, 0xFE95, 0x000C, 0x0001, 0x0004, 0x001E, 0x0000, 0xFE96, 0xFFCF, 0x0001, 0x0004, 0x003C, 0x0001, 0x0000 };
PersonData stg0198PlacesEne13CParam[] = {
    { 467, (u32) &stg0198PlacesEne13CParamPath, { 5, 8 }, 0, 0.0f, 0 },
};
u16 stg0198PlacesEne13BParamPath[] = { 0x0000, 0xFFAD, 0x000F, 0x0001, 0x0000, 0xFF6E, 0xFFF1, 0x0001, 0x0003, 0x0000, 0x0002, 0x012C, 0x0004, 0x0014, 0x0002, 0x0064, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData stg0198PlacesEne13BParam[] = {
    { 468, (u32) &stg0198PlacesEne13BParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
u16 stg0198PlacesEne138Param21Path[] = { 0x0002, 0x0014, 0x0004, 0x0028, 0x0002, 0x0154, 0x0004, 0x0028, 0x0001, 0x0000, 0x0001, 0x0000 };
PersonData stg0198PlacesEne138Param21[] = {
    { 473, (u32) &stg0198PlacesEne138Param21Path, { 5, 8 }, 2, 0.0f, 0 },
};
u16 stg0198PlacesEne139ParamPath[] = { 0x0009, 0x000A, 0x0000, 0x0017, 0x001B, 0x0005, 0x0004, 0x0028, 0x0000, 0xFF02, 0x0016, 0x0005, 0x0001, 0x0000 };
PersonData stg0198PlacesEne139Param[] = {
    { 470, (u32) &stg0198PlacesEne139ParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
u16 stg0198PlacesEne13AParamPath[] = { 0x0002, 0x00B4, 0x0001, 0x0000 };
PersonData stg0198PlacesEne13AParam[] = {
    { 471, (u32) &stg0198PlacesEne13AParamPath, { 5, 8 }, 0, 1.0f, 0 },
};
u16 stg0198PlacesEne13EParamPath[] = { 0x0009, 0x000A, 0x0000, 0x0052, 0x0003, 0x0004, 0x0000, 0x0052, 0xFFB8, 0x0004, 0x0000, 0x0093, 0xFFB8, 0x0004, 0x0000, 0x0093, 0x0003, 0x0004, 0x0001, 0x0000 };
PersonData stg0198PlacesEne13EParam[] = {
    { 472, (u32) &stg0198PlacesEne13EParamPath, { 5, 8 }, 0, 0.0f, 0 },
};
u16 stg0198PlacesEne138Param25Path[] = { 0x0000, 0x012C, 0x001E, 0x0002, 0x0002, 0x00B4, 0x0004, 0x0028, 0x0000, 0x0190, 0x001E, 0x0002, 0x0002, 0x010E, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData stg0198PlacesEne138Param25[] = {
    { 469, (u32) &stg0198PlacesEne138Param25Path, { 5, 8 }, 2, 0.0f, 0 },
};
u16 stg0198PlacesEne13FParamPath[] = { 0x0002, 0x012C, 0x0004, 0x0014, 0x0002, 0x001E, 0x0004, 0x0014, 0x0002, 0x0000, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData stg0198PlacesEne13FParam[] = {
    { 474, (u32) &stg0198PlacesEne13FParamPath, { 5, 8 }, 2, 0.0f, 0 },
};
ConnectData stg0198PlacesEne0FAParam0[] = {
    { 15.0f, 25, 0, -910.0f, 0.0f, -140.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData stg0198PlacesEne0FAParam1[] = {
    { 7.0f, 163, 0, -47.0f, 0.0f, 73.0f, 2.5132742f, 0x0000, 0, 0 },
};
ConnectData stg0198PlacesEne0FAParam2[] = {
    { 7.0f, 164, 0, -47.0f, 0.0f, 73.0f, 2.5132742f, 0x0000, 0, 0 },
};
ConnectData stg0198PlacesEne0FAParam3[] = {
    { 7.0f, 165, 0, -47.0f, 0.0f, 73.0f, 2.5132742f, 0x0000, 0, 0 },
};
ConnectData stg0198PlacesEne0FAParam4[] = {
    { 7.0f, 166, 0, -47.0f, 0.0f, 73.0f, 2.5132742f, 0x0000, 0, 0 },
};
ConnectData stg0198PlacesEne0FAParam5[] = {
    { 7.0f, 167, 0, -47.0f, 0.0f, 73.0f, 2.5132742f, 0x0000, 0, 0 },
};
ConnectData stg0198PlacesEne0FAParam6[] = {
    { 7.0f, 168, 0, -47.0f, 0.0f, 73.0f, 2.5132742f, 0x0000, 0, 0 },
};
ConnectData stg0198PlacesEne0FAParam7[] = {
    { 7.0f, 169, 0, -47.0f, 0.0f, 73.0f, 2.5132742f, 0x0000, 0, 0 },
};
ConnectData stg0198PlacesEne0FAParam8[] = {
    { 7.0f, 170, 0, -47.0f, 0.0f, 73.0f, 2.5132742f, 0x0000, 0, 0 },
};
DoorsData stg0198PlacesEne19BParam9[] = {
    { 0.0f, 8 },
};
DoorsData stg0198PlacesEne19BParam10[] = {
    { 0.0f, 8 },
};
DoorsData stg0198PlacesEne19BParam11[] = {
    { 0.0f, 8 },
};
DoorsData stg0198PlacesEne19BParam12[] = {
    { 0.0f, 8 },
};
DoorsData stg0198PlacesEne19BParam13[] = {
    { 0.0f, 8 },
};
DoorsData stg0198PlacesEne19BParam14[] = {
    { 0.0f, 8 },
};
DoorsData stg0198PlacesEne19BParam15[] = {
    { 0.0f, 8 },
};
DoorsData stg0198PlacesEne19BParam16[] = {
    { 0.0f, 8 },
};
ConnectData stg0198PlacesEne0FAParam17[] = {
    { 10.0f, 171, 0, 10.0f, 20.0f, -2.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef stg0198Places[] = {
    { 0, -470, 0, 20, ENE_CONNECT, 2, (u32) &stg0198PlacesEne0FAParam0 },
    { 0, -271, 5, -30, ENE_CONNECT, 2, (u32) &stg0198PlacesEne0FAParam1 },
    { 0, -207, 5, -30, ENE_CONNECT, 2, (u32) &stg0198PlacesEne0FAParam2 },
    { 0, -147, 5, -30, ENE_CONNECT, 2, (u32) &stg0198PlacesEne0FAParam3 },
    { 0, -83, 5, -30, ENE_CONNECT, 2, (u32) &stg0198PlacesEne0FAParam4 },
    { 0, -20, 5, -30, ENE_CONNECT, 2, (u32) &stg0198PlacesEne0FAParam5 },
    { 0, 238, 5, -30, ENE_CONNECT, 2, (u32) &stg0198PlacesEne0FAParam6 },
    { 0, 303, 5, -30, ENE_CONNECT, 2, (u32) &stg0198PlacesEne0FAParam7 },
    { 0, 366, 5, -30, ENE_CONNECT, 2, (u32) &stg0198PlacesEne0FAParam8 },
    { 0, -271, 0, -20, ENE_DOORS_SET_96, 2, (u32) &stg0198PlacesEne19BParam9 },
    { 0, -209, 0, -20, ENE_DOORS_SET_96, 2, (u32) &stg0198PlacesEne19BParam10 },
    { 0, -146, 0, -20, ENE_DOORS_SET_96, 2, (u32) &stg0198PlacesEne19BParam11 },
    { 0, -84, 0, -20, ENE_DOORS_SET_96, 2, (u32) &stg0198PlacesEne19BParam12 },
    { 0, -21, 0, -20, ENE_DOORS_SET_96, 2, (u32) &stg0198PlacesEne19BParam13 },
    { 0, 240, 0, -20, ENE_DOORS_SET_96, 2, (u32) &stg0198PlacesEne19BParam14 },
    { 0, 303, 0, -20, ENE_DOORS_SET_96, 2, (u32) &stg0198PlacesEne19BParam15 },
    { 0, 365, 0, -20, ENE_DOORS_SET_96, 2, (u32) &stg0198PlacesEne19BParam16 },
    { 0, 115, 0, -34, ENE_CONNECT, 2, (u32) &stg0198PlacesEne0FAParam17 },
    { 0, 0, -20, 45, ENE_KAWA_KAWA, 2, 2 },
    { 0, -363, 2, 35, ENE_ZISAN, 2, (u32) &stg0198PlacesEne13CParam },
    { 0, -83, 2, 15, ENE_ROUNIN, 2, (u32) &stg0198PlacesEne13BParam },
    { 0, 147, 2, -92, ENE_CHOUNIN, 2, (u32) &stg0198PlacesEne138Param21 },
    { 0, 23, 2, 27, ENE_HIKYAKU, 2, (u32) &stg0198PlacesEne139Param },
    { 0, 45, 2, -80, ENE_SHOUNIN, 2, (u32) &stg0198PlacesEne13AParam },
    { 0, 82, 2, 3, ENE_KODOMO, 2, (u32) &stg0198PlacesEne13EParam },
    { 0, 300, 2, 30, ENE_CHOUNIN, 2, (u32) &stg0198PlacesEne138Param25 },
    { 0, 483, 2, -37, ENE_INU, 2, (u32) &stg0198PlacesEne13FParam },
};

void stg0198Build(void) {
    stageBuildZoneCam(9, 0, -435.0f, 0.0f, 20.0f, (u32) stg0198Places, 0x1B);
    camAiSetFar(600);
    camAiAddLimit(1, 1, 0x3F2);
    camAiAddLimit(1, -1, 0xA5);
    cambgMove(cambgCreate(gStageTask, 2, 400.0f), 0, 0x1C);
    camSunSetTunes4(0x79, 0x76, 0x77, 0x78);
    camSunSetDawnSound(0x42F);
    sndPlay(0x521);
}
