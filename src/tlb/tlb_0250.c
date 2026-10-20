#include "tlb.h"

/* File 250: "UKIYO TOWN" (the stage list, file 13). While event flag 0x29 is set and 0xB6 is
 * not, the town is built with the event's placements and kind 0x1C8 is made. */

void ukiyoTownInit(void);
PersonData ukiyoTownPlacesEne1C5Param[] = {
    { 0, 0x00000000, { 8, 8 }, 0, 0.0f, 0 },
};
PersonData ukiyoTownPlacesEventEne1C8Param[] = {
    { 0, 0x00000000, { 8, 8 }, 0, 0.0f, 0 },
};
PersonData ukiyoTownPlacesEventEne1E9Param2[] = {
    { 1, 0x00000000, { 15, 50 }, 2, 0.852887f, 0 },
};
PersonData ukiyoTownPlacesEventEne1E9Param3[] = {
    { 1, 0x00000000, { 15, 50 }, 2, 5.534601f, 0 },
};
PersonData ukiyoTownPlacesEne1C9Param[] = {
    { 0, 0x00000000, { 10, 8 }, 0, 0.0f, 0 },
};
ExitData ukiyoTownPlacesEne0F8Param0[] = {
    { 25.0f, 4, 3, 3, 0 },
};
ExitData ukiyoTownPlacesEne0F8Param1[] = {
    { 25.0f, 4, 3, 3, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam23[] = {
    { 15.0f, 215, 0, 31.0f, 0.0f, 80.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam24[] = {
    { 15.0f, 216, 0, 58.0f, 0.0f, 65.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam27[] = {
    { 10.0f, 226, 0, 4.0f, 0.0f, 70.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam28[] = {
    { 10.0f, 217, 0, 58.0f, 0.0f, 54.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam29[] = {
    { 15.0f, 218, 0, -15.0f, 0.0f, 78.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam30[] = {
    { 15.0f, 218, 0, 44.0f, 0.0f, 78.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam31[] = {
    { 15.0f, 219, 0, -40.0f, 0.0f, 110.0f, 3.1415927f, 0x0007, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam32[] = {
    { 15.0f, 219, 0, 0.0f, 0.0f, 110.0f, 3.1415927f, 0x0007, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam33[] = {
    { 15.0f, 219, 0, 40.0f, 0.0f, 110.0f, 3.1415927f, 0x0007, 0, 0 },
};
/* No place points at this one (stage 0xDC); it lies in the run of the town's connect records. */
ConnectData ukiyoTownUnusedEne0FAParam[] = {
    { 15.0f, 220, 0, 5.0f, 0.0f, 70.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam34[] = {
    { 7.0f, 221, 0, -47.0f, 0.0f, 73.0f, 2.5132742f, 0x0000, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam35[] = {
    { 7.0f, 222, 0, -47.0f, 0.0f, 73.0f, 2.5132742f, 0x0000, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam36[] = {
    { 7.0f, 223, 0, -47.0f, 0.0f, 73.0f, 2.5132742f, 0x0000, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam37[] = {
    { 15.0f, 224, 0, 5.0f, 0.0f, 70.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam38[] = {
    { 15.0f, 225, 0, 11.0f, 0.0f, 120.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam39[] = {
    { 15.0f, 227, 0, 38.0f, 0.0f, 70.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam40[] = {
    { 10.0f, 228, 0, 10.0f, 20.0f, -2.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData ukiyoTownPlacesEne0FAParam41[] = {
    { 10.0f, 229, 0, 10.0f, 20.0f, -2.0f, 0.0f, 0x0000, 0, 0 },
};
DoorsData ukiyoTownPlacesCommonEne19BParam0[] = {
    { 5.141901f, 8 },
};
DoorsData ukiyoTownPlacesCommonEne19BParam1[] = {
    { 5.902757f, 9 },
};
DoorsData ukiyoTownPlacesCommonEne19BParam2[] = {
    { 0.349748f, 0 },
};
extern EneDef ukiyoTownPlaces[];
extern EneDef ukiyoTownPlacesCommon[];
extern EneDef ukiyoTownPlacesEvent[];

void ukiyoTownInit(void) {
    gGameWork.geoCacheSize = 45;
    if (swWorkTest(0x29) && !swWorkTest(0xB6)) {
        stageBuildZoneCam2(0x36, 0, -665.0f, 0.0f, 135.0f, (u32) ukiyoTownPlacesCommon, 9, (u32) ukiyoTownPlacesEvent, 0x11);
    } else {
        stageBuildZoneCam2(0x36, 0, -665.0f, 0.0f, 135.0f, (u32) ukiyoTownPlaces, 0x31, (u32) ukiyoTownPlacesCommon, 9);
    }
    camAiSetFar(500);
    camSunSetProgram(7);
    if (swWorkTest(0x29) && !swWorkTest(0xB6)) {
        PRINTF("*********   st110 イベント用セット OK\n");
        sndPlay(0xBA);
    } else if (gSaveData.allClear == 0) {
        if (!swWorkTest(0x2E) || swWorkTest(0xB3)) {
            camSunSetTunes4(0xBE, 0xBB, 0xBC, 0xBD);
            camSunSetDawnSound(0x42F);
            PRINTF(">>>>>>>>>>>***debug   ゲームクリア前  通常サウンド\n");
        }
    } else {
        if (!swWorkTest(0x2F) || swWorkTest(0xB7)) {
            camSunSetTunes4(0xBE, 0xBB, 0xBC, 0xBD);
            camSunSetDawnSound(0x42F);
            /* sic: the same "before the game is cleared" text as above */
            PRINTF(">>>>>>>>>>>***debug   ゲームクリア前  通常サウンド\n");
        }
    }
    sndPlay(0x521);
    if (swWorkTest(0x29) && !swWorkTest(0xB6)) {
        eneMakeEx(NULL, ENE_DTSK_HIKYAKU, 0, -583.0f, 2.0f, 320.0f, 2, (u32 (*)(void *, void *, void *)) ukiyoTownPlacesEventEne1C8Param);
    }
}

/* The place tables and their records are .data: they run on from ukiyoTownPlacesEne1C5Param to
 * 0x08000DB4 (the .data section's end, padded to 0x08000DC0), and the file's .rodata is only
 * ukiyoTownInit's three print strings at 0x08000DC0. */
DoorsData ukiyoTownPlacesCommonEne19BParam3[] = {
    { 5.528464f, 3 },
};
DoorsData ukiyoTownPlacesCommonEne19BParam4[] = {
    { 6.123651f, 4 },
};
DoorsData ukiyoTownPlacesCommonEne19BParam5[] = {
    { 6.142059f, 4 },
};
DoorsData ukiyoTownPlacesCommonEne19BParam6[] = {
    { 6.123651f, 4 },
};
DoorsData ukiyoTownPlacesCommonEne19BParam7[] = {
    { 0.220893f, 3 },
};
DoorsData ukiyoTownPlacesCommonEne19BParam8[] = {
    { 0.779263f, 1 },
};
u16 ukiyoTownPlacesEne13DParam2Path[] = { 0x0002, 0x005A, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne13DParam2[] = {
    { 682, (u32) ukiyoTownPlacesEne13DParam2Path, { 5, 8 }, 2, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne13EParam3Path[] = { 0x0000, 0xFD7B, 0x010A, 0x0003, 0x0000, 0xFD82, 0x00D1, 0x0003, 0x0004, 0x001E, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne13EParam3[] = {
    { 689, (u32) ukiyoTownPlacesEne13EParam3Path, { 5, 8 }, 0, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne139Param4Path[] = { 0x0000, 0xFDAB, 0x012F, 0x0003, 0x0000, 0xFE32, 0xFE14, 0x0003, 0x0004, 0x001E, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne139Param4[] = {
    { 692, (u32) ukiyoTownPlacesEne139Param4Path, { 5, 8 }, 4, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne13CParam5Path[] = { 0x0000, 0x0000, 0x01EA, 0x0001, 0x0004, 0x001E, 0x0000, 0x0000, 0x028B, 0x0001, 0x0004, 0x001E, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne13CParam5[] = {
    { 687, (u32) ukiyoTownPlacesEne13CParam5Path, { 5, 8 }, 2, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne13BParam6Path[] = { 0x0003, 0x0000, 0x0002, 0x012C, 0x0004, 0x0014, 0x0002, 0x0064, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne13BParam6[] = {
    { 694, (u32) ukiyoTownPlacesEne13BParam6Path, { 5, 8 }, 2, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne13AParam7Path[] = { 0x0000, 0x01EF, 0x019B, 0x0002, 0x0004, 0x0014, 0x0000, 0x020B, 0x01D5, 0x0002, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne13AParam7[] = {
    { 696, (u32) ukiyoTownPlacesEne13AParam7Path, { 5, 8 }, 2, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne13DParam8Path[] = { 0x0002, 0x0014, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne13DParam8[] = {
    { 683, (u32) ukiyoTownPlacesEne13DParam8Path, { 5, 8 }, 2, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne138Param9Path[] = { 0x0002, 0x0014, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne138Param9[] = {
    { 698, (u32) ukiyoTownPlacesEne138Param9Path, { 5, 8 }, 4, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne13FParam10Path[] = { 0x0002, 0x012C, 0x0004, 0x0014, 0x0002, 0x001E, 0x0004, 0x0014, 0x0002, 0x0000, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne13FParam10[] = {
    { 700, (u32) ukiyoTownPlacesEne13FParam10Path, { 5, 8 }, 2, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne13BParam11Path[] = { 0x0002, 0x0078, 0x0004, 0x0014, 0x0002, 0x00D2, 0x0004, 0x0014, 0x0000, 0x04F4, 0x0186, 0x0002, 0x0004, 0x0014, 0x0000, 0x04D3, 0x0276, 0x0002, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne13BParam11[] = {
    { 695, (u32) ukiyoTownPlacesEne13BParam11Path, { 5, 8 }, 2, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne140Param12Path[] = { 0x0002, 0x00B4, 0x0004, 0x0014, 0x0002, 0x0000, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne140Param12[] = {
    { 685, (u32) ukiyoTownPlacesEne140Param12Path, { 5, 8 }, 1, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne139Param13Path[] = { 0x0000, 0x0681, 0x0021, 0x0008, 0x0000, 0x066F, 0x00DD, 0x0008, 0x0004, 0x003C, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne139Param13[] = {
    { 693, (u32) ukiyoTownPlacesEne139Param13Path, { 5, 8 }, 2, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne13CParam14Path[] = { 0x0002, 0x0000, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne13CParam14[] = {
    { 688, (u32) ukiyoTownPlacesEne13CParam14Path, { 5, 8 }, 4, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne13AParam15Path[] = { 0x0002, 0x010E, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne13AParam15[] = {
    { 697, (u32) ukiyoTownPlacesEne13AParam15Path, { 5, 8 }, 2, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne13EParam16Path[] = { 0x0000, 0xFF7E, 0x0410, 0x0003, 0x0000, 0xFEBE, 0x03EF, 0x0003, 0x0000, 0xFEFE, 0x03D9, 0x0003, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne13EParam16[] = {
    { 690, (u32) ukiyoTownPlacesEne13EParam16Path, { 5, 8 }, 0, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne140Param17Path[] = { 0x0002, 0x00D2, 0x0004, 0x0014, 0x0002, 0x0096, 0x0004, 0x0014, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne140Param17[] = {
    { 686, (u32) ukiyoTownPlacesEne140Param17Path, { 5, 8 }, 4, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne13EParam18Path[] = { 0x0000, 0xFF12, 0x0540, 0x0004, 0x0000, 0xFF4E, 0x054A, 0x0004, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne13EParam18[] = {
    { 691, (u32) ukiyoTownPlacesEne13EParam18Path, { 5, 8 }, 0, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne13FParam19Path[] = { 0x0002, 0x0000, 0x0004, 0x000A, 0x0002, 0x003C, 0x0004, 0x000A, 0x0002, 0x0078, 0x0004, 0x000A, 0x0002, 0x00B4, 0x0004, 0x000A, 0x0002, 0x00F0, 0x0004, 0x000A, 0x0002, 0x012C, 0x0004, 0x000A, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne13FParam19[] = {
    { 701, (u32) ukiyoTownPlacesEne13FParam19Path, { 5, 8 }, 2, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne138Param20Path[] = { 0x0002, 0x00B4, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne138Param20[] = {
    { 699, (u32) ukiyoTownPlacesEne138Param20Path, { 5, 8 }, 2, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne13FParam21Path[] = { 0x0000, 0xFB7B, 0x04BB, 0x0002, 0x0000, 0xFE34, 0x0640, 0x0002, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne13FParam21[] = {
    { 702, (u32) ukiyoTownPlacesEne13FParam21Path, { 5, 8 }, 4, 0.0f, 0 },
};
u16 ukiyoTownPlacesEne13DParam22Path[] = { 0x0002, 0x00B4, 0x0001, 0x0000 };
PersonData ukiyoTownPlacesEne13DParam22[] = {
    { 684, (u32) ukiyoTownPlacesEne13DParam22Path, { 5, 8 }, 1, 0.0f, 0 },
};

/* The town's 0x31 places (four words each); the fourth word is each one's argument. */
EneDef ukiyoTownPlaces[] = {
    { 0, -650, -5, 86, ENE_EXIT, 2, (u32) ukiyoTownPlacesEne0F8Param0 },
    { 0, 657, -3, 89, ENE_EXIT, 2, (u32) ukiyoTownPlacesEne0F8Param1 },
    { 0, -687, 2, 114, ENE_DERO_KARASU_YAKUNIN, 2, (u32) ukiyoTownPlacesEne13DParam2 },
    { 0, -638, 2, 209, ENE_KODOMO, 2, (u32) ukiyoTownPlacesEne13EParam3 },
    { 0, -462, 2, -492, ENE_HIKYAKU, 2, (u32) ukiyoTownPlacesEne139Param4 },
    { 0, 0, 2, 651, ENE_ZISAN, 2, (u32) ukiyoTownPlacesEne13CParam5 },
    { 0, 326, 2, 621, ENE_ROUNIN, 2, (u32) ukiyoTownPlacesEne13BParam6 },
    { 0, 523, 2, 469, ENE_SHOUNIN, 2, (u32) ukiyoTownPlacesEne13AParam7 },
    { 0, 687, 2, 131, ENE_DERO_KARASU_YAKUNIN, 2, (u32) ukiyoTownPlacesEne13DParam8 },
    { 0, 354, 35, 968, ENE_CHOUNIN, 2, (u32) ukiyoTownPlacesEne138Param9 },
    { 0, 707, 2, 988, ENE_INU, 2, (u32) ukiyoTownPlacesEne13FParam10 },
    { 0, 1235, 2, 630, ENE_ROUNIN, 2, (u32) ukiyoTownPlacesEne13BParam11 },
    { 0, 1162, 2, 117, ENE_MUSUME, 2, (u32) ukiyoTownPlacesEne140Param12 },
    { 0, 1660, 2, 167, ENE_HIKYAKU, 2, (u32) ukiyoTownPlacesEne139Param13 },
    { 0, 59, 2, 1632, ENE_ZISAN, 2, (u32) ukiyoTownPlacesEne13CParam14 },
    { 0, -19, 2, 1180, ENE_SHOUNIN, 2, (u32) ukiyoTownPlacesEne13AParam15 },
    { 0, -258, 2, 985, ENE_KODOMO, 2, (u32) ukiyoTownPlacesEne13EParam16 },
    { 0, -335, 2, 914, ENE_MUSUME, 2, (u32) ukiyoTownPlacesEne140Param17 },
    { 0, -178, 2, 1354, ENE_KODOMO, 2, (u32) ukiyoTownPlacesEne13EParam18 },
    { 0, -776, 30, 1127, ENE_INU, 2, (u32) ukiyoTownPlacesEne13FParam19 },
    { 0, -971, 2, 879, ENE_CHOUNIN, 2, (u32) ukiyoTownPlacesEne138Param20 },
    { 0, -460, 2, 1600, ENE_INU, 2, (u32) ukiyoTownPlacesEne13FParam21 },
    { 0, -1360, 2, 104, ENE_DERO_KARASU_YAKUNIN, 2, (u32) ukiyoTownPlacesEne13DParam22 },
    { 0, -572, 0, 253, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam23 },
    { 0, -218, 0, 599, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam24 },
    { 0, -196, 0, 608, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam24 },
    { 0, -173, 0, 620, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam24 },
    { 0, -156, 4, 645, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam27 },
    { 0, 246, 0, 577, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam28 },
    { 0, 530, 0, 327, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam29 },
    { 0, 564, 0, 271, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam30 },
    { 0, -563, 0, 1185, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam31 },
    { 0, -513, 0, 1207, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam32 },
    { 0, -462, 0, 1227, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam33 },
    { 0, -276, 5, 957, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam34 },
    { 0, -217, 4, 968, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam35 },
    { 0, -155, 5, 979, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam36 },
    { 0, 176, 0, 969, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam37 },
    { 0, 692, 5, 779, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam38 },
    { 0, 119, 5, 1325, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam39 },
    { 0, -63, 2, 964, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam40 },
    { 0, 484, 0, 813, ENE_CONNECT, 2, (u32) ukiyoTownPlacesEne0FAParam41 },
    { 0, 0, 0, 0, ENE_SHOUNIN_ONIBI, 0, 0 },
    { 0, -1267, 0, 183, ENE_SHOUNIN_GHOST, 2, (u32) ukiyoTownPlacesEne1C5Param },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, -917, 0, 761, ENE_DTSK_ZUKIN_MON, 2, (u32) ukiyoTownPlacesEne1C9Param },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
};

/* The 9 places built in both cases. */
EneDef ukiyoTownPlacesCommon[] = {
    { 0, -590, 0, 262, ENE_DOORS_SET_96, 2, (u32) ukiyoTownPlacesCommonEne19BParam0 },
    { 0, -157, 1, 647, ENE_DOORS_SET_96, 2, (u32) ukiyoTownPlacesCommonEne19BParam1 },
    { 0, 248, 1, 578, ENE_DOORS_SET_96, 2, (u32) ukiyoTownPlacesCommonEne19BParam2 },
    { 0, -906, 0, 749, ENE_DOORS_SET_96, 2, (u32) ukiyoTownPlacesCommonEne19BParam3 },
    { 0, -279, 0, 966, ENE_DOORS_SET_96, 2, (u32) ukiyoTownPlacesCommonEne19BParam4 },
    { 0, -217, 0, 976, ENE_DOORS_SET_96, 2, (u32) ukiyoTownPlacesCommonEne19BParam5 },
    { 0, -156, 0, 987, ENE_DOORS_SET_96, 2, (u32) ukiyoTownPlacesCommonEne19BParam6 },
    { 0, 175, 0, 988, ENE_DOORS_SET_96, 2, (u32) ukiyoTownPlacesCommonEne19BParam7 },
    { 0, 699, 0, 798, ENE_DOORS_SET_96, 2, (u32) ukiyoTownPlacesCommonEne19BParam8 },
};

/* Unreferenced here: a record of the same shape as ukiyoTownPlacesEventEne1E9Param2's. */
u32 ukiyoTownUnusedKindData[] = { 0x00000001, 0x00000000, 0x0F320200, 0x3E623140, 0x00000000 };

/* The event's 0x11 places. */
EneDef ukiyoTownPlacesEvent[] = {
    { 0, 0, 0, 0, ENE_DTSK_HIKYAKU, 2, (u32) ukiyoTownPlacesEventEne1C8Param },
    { 0, 0, 0, 0, ENE_DEMO_TIMER, 0, 0 },
    { 0, 658, 2, 123, ENE_HIKYAKU_JIZOU, 2, (u32) ukiyoTownPlacesEventEne1E9Param2 },
    { 0, -652, 2, 109, ENE_HIKYAKU_JIZOU, 2, (u32) ukiyoTownPlacesEventEne1E9Param3 },
    { 0, -587, 0, 260, ENE_DEMO_JIZOU_FILE_138, 2, 7690 },
    { 0, -153, 0, 643, ENE_DEMO_JIZOU_FILE_138, 2, 7690 },
    { 0, 245, 0, 573, ENE_DEMO_JIZOU_FILE_138, 2, 7690 },
    { 0, -277, 0, 964, ENE_DEMO_JIZOU_FILE_138, 2, 7690 },
    { 0, -216, 0, 972, ENE_DEMO_JIZOU_FILE_138, 2, 7690 },
    { 0, -158, 0, 980, ENE_DEMO_JIZOU_FILE_138, 2, 7690 },
    { 0, 173, 0, 982, ENE_DEMO_JIZOU_FILE_138, 2, 7690 },
    { 0, 188, 0, 981, ENE_DEMO_JIZOU_FILE_138, 2, 7690 },
    { 0, 696, 0, 792, ENE_DEMO_JIZOU_FILE_138, 2, 7690 },
    { 0, -561, 0, 1173, ENE_DEMO_JIZOU_FILE_138, 2, 7690 },
    { 0, -510, 0, 1192, ENE_DEMO_JIZOU_FILE_138, 2, 7690 },
    { 0, -460, 0, 1214, ENE_DEMO_JIZOU_FILE_138, 2, 7690 },
    { 0, -898, 0, 744, ENE_DEMO_JIZOU_FILE_138, 2, 7690 },
};
