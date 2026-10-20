#include "tlb.h"

extern ConnectData stage76PlacesEne0FAParam[];
extern ExitData stage76PlacesEne0F9Param[];
extern ConnectData stage77PlacesEne0FAParam0[];
extern ConnectData stage77PlacesEne0FAParam1[];
extern ConnectData stage81PlacesEne0FAParam0[];
extern ConnectData stage81PlacesEne0FAParam1[];
extern ConnectData stage81PlacesEne0FAParam2[];
extern ConnectData stage85PlacesEne0FAParam0[];
extern ConnectData stage85PlacesEne0FAParam1[];
extern ConnectData stage85PlacesEne0FAParam2[];
extern ConnectData stage93PlacesEne0FAParam8[];
extern ConnectData stage93PlacesEne0FAParam9[];
extern ConnectData stage94PlacesEne0FAParam0[];
extern ConnectData stage94PlacesEne0FAParam1[];
extern ConnectData stage86PlacesEne0FAParam0[];
extern ConnectData stage86PlacesEne0FAParam1[];
extern WsSound2Data stage86PlacesEne05CParam[];
extern WsSound2Data stage86PlacesEne05BParam[];
ConnectData stage76PlacesEne0FAParam[] = {
    { 30.0f, 73, 1, 110.0f, 0.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ExitData stage76PlacesEne0F9Param[] = {
    { 60.0f, 2, 6, -1, 0 },
};
EneDef stage76Places[] = {
    { 1, 9, 0, 0, ENE_TANU_WALL_FLAGS_0, 0, 0 },
    { 1, 0, 5, 0, ENE_EXIT_ESCAPE, 2, (u32) &stage76PlacesEne0F9Param },
    { 1, 2948, -33, 0, ENE_CONNECT, 2, (u32) &stage76PlacesEne0FAParam },
    { 0, -808, 59, 122, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -908, 59, 172, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -1008, 44, 122, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -1108, 27, 173, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 1332, -21, 252, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 1232, -1, 292, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -642, 39, 57, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 57, 44, -258, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -123, 54, -238, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -208, 39, -168, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -318, 54, -193, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -502, 39, -83, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -537, 39, 22, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 1132, 14, 247, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 344, 109, -193, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 273, 89, -242, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 182, 64, -259, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -418, 59, -154, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -711, 59, 123, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -1208, 29, 122, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 792, 99, 185, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 967, 54, 222, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 697, 129, 132, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 567, 144, 32, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 462, 124, -3, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 423, 109, -108, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 860, 79, 194, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_TEAR, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_CLOUD, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_SMOKE, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_ONO, 0, 0 },
    { 1, 345, 50, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 415, 60, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 485, 50, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 575, 50, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 655, 40, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 740, 50, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 825, 40, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 930, 55, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 1080, 60, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 1160, 50, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 1230, 50, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 1330, 70, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 1450, 65, 0, ENE_MONOEYE, 2, 1 },
    { 1, 1548, 60, 0, ENE_MONOEYE, 2, 1 },
    { 1, 1627, 80, 0, ENE_MONOEYE, 2, 1 },
    { 1, 1700, 100, 0, ENE_MONOEYE, 2, 1 },
    { 1, 1880, 90, 0, ENE_MOUJA, 2, 1 },
    { 1, 1975, 97, 0, ENE_MOUJA, 2, 1 },
    { 1, 2080, 120, 0, ENE_MOUJA, 2, 1 },
    { 1, 2210, 107, 0, ENE_MOUJA, 2, 1 },
    { 1, 2260, 90, 0, ENE_MOUJA, 2, 1 },
    { 1, 2340, 115, 0, ENE_MONOEYE, 2, 1 },
    { 1, 2415, 100, 0, ENE_MONOEYE, 2, 1 },
    { 1, 2500, 27, 0, ENE_MOUJA, 2, 1 },
    { 1, 2550, 27, 0, ENE_MOUJA, 2, 1 },
    { 1, 2695, -11, 0, ENE_MOUJA, 2, 1 },
};
ConnectData stage77PlacesEne0FAParam0[] = {
    { 30.0f, 72, 1, 2905.0f, -47.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData stage77PlacesEne0FAParam1[] = {
    { 30.0f, 74, 1, 140.0f, 5.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef stage77Places[] = {
    { 1, 70, 30, 0, ENE_CONNECT, 2, (u32) &stage77PlacesEne0FAParam0 },
    { 1, 3955, 30, 0, ENE_CONNECT, 2, (u32) &stage77PlacesEne0FAParam1 },
    { 1, 2493, 120, 0, ENE_ITEM_OOIRI_MAIN, 2, 409 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 1, 870, 25, 0, ENE_TRANSFER, 2, 0 },
    { 1, 3000, 80, 0, ENE_ETSK_GOLDNEKO_MAIN_FLAGS_0, 2, 0 },
    { 1, 2030, 16, 0, ENE_ITEM_KOREZO_MAIN, 2, 0 },
    { 0, 1300, 31, 73, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, 969, 36, -224, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, 596, 36, -291, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -983, 36, -284, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -1215, 36, -284, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -204, 36, -284, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -1485, 36, -185, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -1490, 35, 275, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -1881, 36, -184, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, 17, 10, -238, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -507, 36, -284, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, 1602, 36, 569, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, 1014, -18, -104, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, 1680, 35, 469, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 1, 1284, 0, 0, ENE_IDASHI, 2, 60 },
    { 1, 2298, 0, 0, ENE_IDASHI, 2, 60 },
    { 1, 3472, 0, 0, ENE_IDASHI, 2, 60 },
    { 0, -64, 38, -240, ENE_BPOLE, 2, 0 },
    { 0, -135, 50, -240, ENE_BPOLE, 2, 0 },
    { 0, -144, 80, 310, ENE_BPOLE, 2, -128 },
    { 0, 555, 25, -240, ENE_BPOLE, 2, 0 },
    { 0, 500, 52, -240, ENE_BPOLE, 2, 0 },
    { 0, 450, 24, -240, ENE_BPOLE, 2, 0 },
    { 0, 500, 0, -240, ENE_ARIJIGOKU, 2, 0 },
    { 0, 500, -20, -240, ENE_INSECT, 2, 0 },
    { 0, 1030, 0, -90, ENE_ARIJIGOKU, 2, -128 },
    { 0, 1170, 0, 50, ENE_ARIJIGOKU, 2, -128 },
    { 0, 1170, -20, 50, ENE_INSECT, 2, -128 },
    { 0, 1310, 0, 190, ENE_ARIJIGOKU, 2, -128 },
    { 0, 1310, -20, 190, ENE_INSECT, 2, -128 },
    { 0, 1450, 0, 330, ENE_ARIJIGOKU, 2, -128 },
    { 0, 1450, -20, 330, ENE_INSECT, 2, -128 },
    { 0, 300, 0, -240, ENE_ARIJIGOKU, 2, 0 },
    { 0, 300, -20, -240, ENE_INSECT, 2, 0 },
    { 0, -900, 0, -240, ENE_ARIJIGOKU, 2, 0 },
    { 0, -900, -20, -240, ENE_INSECT, 2, 0 },
    { 0, -700, 0, -240, ENE_ARIJIGOKU, 2, 0 },
    { 0, -700, -20, -240, ENE_INSECT, 2, 0 },
    { 0, -1700, 0, -140, ENE_ARIJIGOKU, 2, 0 },
    { 0, -100, 0, -240, ENE_ARIJIGOKU, 2, 0 },
    { 0, -100, -20, -240, ENE_INSECT, 2, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_TEAR, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_CLOUD, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_SMOKE, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_ONO, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_KOZ, 0, 0 },
    { 0, 0, 0, 0, ENE_NORU_MARK, 0, 0 },
    { 1, 270, 37, 0, ENE_NUPPE, 2, 0 },
    { 1, 390, 35, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 470, 50, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 650, 7, 0, ENE_MOUJA, 2, 1 },
    { 1, 705, 7, 0, ENE_NUPPE, 2, 0 },
    { 1, 775, 7, 0, ENE_MOUJA, 2, 1 },
    { 1, 950, 35, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 1030, 35, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 1130, 35, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 1230, 35, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 1330, 35, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 1515, 7, 0, ENE_MOUJA, 2, 65537 },
    { 1, 1610, 21, 0, ENE_MOUJA, 2, 65537 },
    { 1, 1710, 37, 0, ENE_MOUJA, 2, 65537 },
    { 1, 2207, 18, 0, ENE_MOUJA, 2, 0 },
    { 1, 2080, 7, 0, ENE_MOUJA, 2, 65537 },
    { 1, 2120, 7, 0, ENE_MOUJA, 2, 65537 },
    { 1, 2630, 7, 0, ENE_MOUJA, 2, 0 },
    { 1, 2650, 7, 0, ENE_MOUJA, 2, 0 },
    { 1, 2700, 7, 0, ENE_SENSYA_KOZURE, 0, 0 },
    { 1, 2760, 7, 0, ENE_MOUJA, 2, 0 },
    { 1, 2790, 7, 0, ENE_MOUJA, 2, 0 },
    { 1, 2820, 7, 0, ENE_MOUJA, 2, 0 },
    { 1, 2890, 7, 0, ENE_MOUJA, 2, 0 },
    { 1, 2930, 7, 0, ENE_MOUJA, 2, 0 },
    { 1, 3270, 92, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 3340, 78, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 3374, 19, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 3405, 77, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 3445, 31, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 3499, 61, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 3570, 27, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 3608, 74, 0, ENE_HELLBAT, 2, 1280 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_POT_FRAGMENT, 0, 0 },
    { 1, 540, 34, 0, ENE_POT, 2, 0 },
    { 1, 1420, 25, 0, ENE_POT, 2, 0 },
    { 1, 1800, 22, 0, ENE_POT, 2, 0 },
    { 1, 3000, 82, 0, ENE_POT, 2, 0 },
    { 1, 3030, 82, 0, ENE_POT, 2, 0 },
    { 1, 3165, 82, 0, ENE_POT, 2, 0 },
};
ConnectData stage81PlacesEne0FAParam0[] = {
    { 30.0f, 73, 1, 3900.0f, 5.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData stage81PlacesEne0FAParam1[] = {
    { 30.0f, 75, 1, 100.0f, -50.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData stage81PlacesEne0FAParam2[] = {
    { 30.0f, 75, 2, 110.0f, 105.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef stage81Places[] = {
    { 1, 100, 5, 0, ENE_CONNECT, 2, (u32) &stage81PlacesEne0FAParam0 },
    { 1, 1625, -30, 0, ENE_CONNECT, 2, (u32) &stage81PlacesEne0FAParam1 },
    { 2, 690, 150, 0, ENE_CONNECT, 2, (u32) &stage81PlacesEne0FAParam2 },
    { 1, 636, -60, 0, ENE_TRANSFER, 2, 0 },
    { 0, 593, -20, 368, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, 502, -42, 176, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, 321, -24, 33, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -77, -24, 33, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -232, -24, -35, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -702, 15, -45, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -442, 15, -45, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 1, 1277, 13, 0, ENE_BPOLE, 2, 0 },
    { 1, 1277, -40, 0, ENE_BPOLE, 2, 0 },
    { 0, 268, 112, 53, ENE_HMITA, 2, 0 },
    { 0, 449, 135, 51, ENE_HMITA, 2, 0 },
    { 0, -258, -57, 0, ENE_OBJ22_ELEVATOR22, 2, 60 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_TEAR, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_CLOUD, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_SMOKE, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_ONO, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_KOZ, 0, 0 },
    { 0, 0, 0, 0, ENE_NORU_MARK, 0, 0 },
    { 0, 0, 0, 0, ENE_NECKBITE_TAMA_MAIN, 0, 0 },
    { 1, 300, 18, 0, ENE_NUPPE, 2, 0 },
    { 1, 515, 6, 0, ENE_NUPPE, 2, 0 },
    { 1, 400, 9, 0, ENE_NUPPE, 2, 0 },
    { 1, 717, -40, -30, ENE_DEADHAND, 2, 1 },
    { 1, 759, -43, -31, ENE_DEADHAND, 2, 1 },
    { 1, 816, -40, -32, ENE_DEADHAND, 2, 1 },
    { 1, 905, -42, -28, ENE_DEADHAND, 2, 1 },
    { 1, 1085, -51, -38, ENE_DEADHAND, 2, 1 },
    { 1, 1148, -52, -36, ENE_DEADHAND, 2, 1 },
    { 1, 1249, -42, -36, ENE_DEADHAND, 2, 1 },
    { 1, 1348, -38, -36, ENE_DEADHAND, 2, 1 },
    { 1, 1130, -58, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 1, 1420, -58, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 2, 159, 78, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 2, 331, 122, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 763, -55, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 1, 961, -55, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 1, 1209, 5, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 1, 1104, 5, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 1, 1209, 5, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 2, 200, 105, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 2, 50, 29, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
};
ConnectData stage85PlacesEne0FAParam0[] = {
    { 30.0f, 74, 1, 1570.0f, -55.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData stage85PlacesEne0FAParam1[] = {
    { 30.0f, 74, 2, 650.0f, 120.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData stage85PlacesEne0FAParam2[] = {
    { 30.0f, 132, 1, 40.0f, 40.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef stage85Places[] = {
    { 1, 50, -25, 0, ENE_CONNECT, 2, (u32) &stage85PlacesEne0FAParam0 },
    { 2, 48, 130, 0, ENE_CONNECT, 2, (u32) &stage85PlacesEne0FAParam1 },
    { 1, 1220, 20, 0, ENE_CONNECT, 2, (u32) &stage85PlacesEne0FAParam2 },
    { 2, 1045, 46, 0, ENE_IMP_OBJ_CUBE, 2, 0 },
    { 0, 105, -7, 43, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, 226, -48, 216, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -80, 8, 45, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -199, -9, 73, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -311, -8, 163, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -438, -8, 165, ENE_TOMOSIBI_FLAGS_0, 2, 10 },
    { 0, -58, -24, 105, ENE_KMACE, 2, 0 },
    { 0, 9, -24, 105, ENE_KMACE, 2, 0 },
    { 0, 282, 78, 181, ENE_KMACE, 2, 1 },
    { 0, -132, 124, -17, ENE_KMACE, 2, 0 },
    { 0, -206, 140, -15, ENE_KMACE, 2, 1 },
    { 0, -270, 103, -20, ENE_KMACE, 2, 65863680 },
    { 0, 304, -4, 183, ENE_HMITA, 2, 0 },
    { 0, 116, 101, 39, ENE_HMITA, 2, 900 },
    { 0, -52, 131, -20, ENE_HMITA, 2, 0 },
    { 0, -344, 101, -51, ENE_HMITA, 2, 940 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_TEAR, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_CLOUD, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_SMOKE, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_ONO, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_KOZ, 0, 0 },
    { 0, 0, 0, 0, ENE_NORU_MARK, 0, 0 },
    { 1, 234, -48, 0, ENE_NUPPE, 2, 0 },
    { 1, 472, -23, 0, ENE_MOUJA, 2, 1 },
    { 1, 518, -9, 0, ENE_MOUJA, 2, 1 },
    { 1, 685, -13, 0, ENE_MOUJA, 2, 1 },
    { 1, 749, -28, 0, ENE_MOUJA, 2, 1 },
    { 1, 819, 20, 0, ENE_MONOEYE, 2, 1 },
    { 1, 1038, 16, 0, ENE_MONOEYE, 2, 1 },
    { 2, 240, 147, 0, ENE_HELLBAT, 2, 1280 },
    { 2, 350, 160, 0, ENE_HELLBAT, 2, 1280 },
    { 2, 418, 160, 0, ENE_HELLBAT, 2, 1280 },
    { 2, 590, 163, 0, ENE_MONOEYE, 2, 1 },
    { 2, 811, 135, 0, ENE_MONOEYE, 2, 1 },
    { 2, 668, 145, 0, ENE_MONOEYE, 2, 1 },
};
ConnectData stage93PlacesEne0FAParam8[] = {
    { 30.0f, 75, 1, 1150.0f, 3.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData stage93PlacesEne0FAParam9[] = {
    { 30.0f, 133, 1, 90.0f, 60.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef stage93Places[] = {
    { 0, 0, 0, 0, ENE_POT_FRAGMENT, 0, 0 },
    { 1, 4130, 1162, 0, ENE_ITEM_OOIRI_MAIN, 2, 410 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 1, 1313, 405, 0, ENE_POT, 2, 0 },
    { 1, 1715, 525, 0, ENE_POT, 2, 0 },
    { 1, 2352, 655, 0, ENE_POT, 2, 0 },
    { 1, 0, 60, 0, ENE_CONNECT, 2, (u32) &stage93PlacesEne0FAParam8 },
    { 1, 4172, 1176, 0, ENE_CONNECT, 2, (u32) &stage93PlacesEne0FAParam9 },
    { 1, 82, 40, 0, ENE_TRANSFER, 2, 0 },
    { 1, 2294, 635, 0, ENE_ITEM_KOREZO_MAIN, 2, 0 },
    { 0, 113, 65, -186, ENE_SASROCK, 2, 925 },
    { 0, 103, 420, -188, ENE_SASROCK, 2, 956 },
    { 0, -161, 675, -104, ENE_SASROCK, 2, 918 },
    { 0, -123, 680, -148, ENE_SASROCK, 2, 892 },
    { 0, 97, 730, -187, ENE_SASROCK, 2, 951 },
    { 0, 45, 1135, -206, ENE_SASROCK, 2, 741 },
    { 0, -171, 1045, -123, ENE_SASROCK, 2, 892 },
    { 0, 5, 895, 210, ENE_SASROCK, 2, 764 },
    { 1, 185, 105, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 161, 120, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 1437, 435, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 1462, 456, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 1546, 472, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 1574, 497, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 2458, 695, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 2513, 706, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 2573, 706, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 2623, 720, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 2740, 750, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 2820, 750, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 3300, 910, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 3357, 917, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 3771, 1060, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 3850, 1080, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 4012, 1153, 0, ENE_HELLBAT, 2, 1536 },
    { 1, 4068, 1156, 0, ENE_HELLBAT, 2, 1536 },
    { 0, 0, 675, 0, ENE_CLASHER, 2, 0 },
    { 0, -3, 893, 215, ENE_CLASHBOARD, 2, -12124158 },
    { 0, -198, 973, 76, ENE_CLASHBOARD, 2, 6815746 },
    { 0, -214, 998, 11, ENE_CLASHBOARD, 2, 6553602 },
    { 0, -106, 1078, -187, ENE_CLASHBOARD, 2, 1966082 },
    { 0, -133, 1059, -160, ENE_CLASHBOARD, 2, 2490370 },
    { 0, 127, 1158, -172, ENE_CLASHBOARD, 2, -2949118 },
    { 0, 154, 838, 146, ENE_CLASHBOARD, 2, -8847358 },
    { 0, 179, 753, -117, ENE_CLASHBOARD, 2, -3604478 },
    { 0, -155, 948, 142, ENE_CLASHBOARD, 2, 8781827 },
    { 0, -112, 943, 180, ENE_CLASHBOARD, 2, -14090237 },
    { 0, 58, 888, 204, ENE_CLASHBOARD, 2, -10813437 },
    { 0, 202, 828, 65, ENE_CLASHBOARD, 2, -6881277 },
    { 0, 203, 778, -61, ENE_CLASHBOARD, 2, -4849661 },
    { 0, -191, 1038, -91, ENE_CLASHBOARD, 2, 4587523 },
    { 0, 100, 728, -191, ENE_CLASHBOARD, 2, -1638397 },
    { 0, 62, 1133, -203, ENE_CLASHBOARD, 2, -983037 },
    { 0, 9, 1128, -212, ENE_CLASHBOARD, 2, 3 },
    { 0, -162, 1043, -136, ENE_CLASHBOARD, 2, 2883587 },
    { 0, 213, 803, -1, ENE_CLASHBOARD, 2, -5898239 },
    { 0, 165, 748, -136, ENE_CLASHBOARD, 2, -2949119 },
    { 0, 42, 728, -198, ENE_CLASHBOARD, 2, 983041 },
    { 0, 1, 718, -193, ENE_CLASHBOARD, 2, 1 },
    { 0, 185, 853, 109, ENE_CLASHBOARD, 2, -7929855 },
    { 0, -43, 1103, -209, ENE_CLASHBOARD, 2, 917506 },
    { 0, -208, 1023, -35, ENE_CLASHBOARD, 2, 4849665 },
    { 0, -59, 918, 203, ENE_CLASHBOARD, 2, -12845055 },
    { 0, 115, 863, 183, ENE_CLASHBOARD, 2, -9895934 },
    { 0, 147, 729, -152, ENE_CLASHBOARD, 2, -3276799 },
    { 0, -10, 199, 216, ENE_CLASHBOARD, 2, -12124158 },
    { 0, 208, 108, -60, ENE_HMITA, 2, 813 },
    { 0, -207, 270, 61, ENE_HMITA, 2, 307 },
    { 0, -60, 586, 204, ENE_HMITA, 2, 478 },
    { 0, -155, 611, 146, ENE_HMITA, 2, 383 },
    { 0, 174, 120, 130, ENE_PPRESS, 2, 43475968 },
    { 0, 77, 150, 202, ENE_PPRESS, 2, 37446656 },
    { 0, -160, 320, -143, ENE_PPRESS, 2, 9069568 },
    { 0, 22, 393, -213, ENE_PPRESS, 2, 65954816 },
    { 0, 208, 500, 42, ENE_PPRESS, 2, 48325632 },
    { 0, 170, 524, 129, ENE_PPRESS, 2, 43475968 },
    { 0, 139, 524, 162, ENE_PPRESS, 2, 41182208 },
    { 0, 76, 524, 198, ENE_PPRESS, 2, 37512192 },
    { 0, 34, 534, 210, ENE_PPRESS, 2, 35415040 },
    { 0, 207, 786, 24, ENE_PPRESS, 2, 48849920 },
    { 0, 168, 820, 127, ENE_PPRESS, 2, 43082752 },
    { 0, 132, 820, 164, ENE_PPRESS, 2, 41706496 },
    { 0, -35, 867, 207, ENE_PPRESS, 2, 31941632 },
    { 0, -206, 957, 42, ENE_PPRESS, 2, 19883008 },
    { 0, -71, 1063, -200, ENE_PPRESS, 2, 4416512 },
    { 0, -62, 1113, -228, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 169, 873, 167, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 62, 903, 229, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -62, 933, 229, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -167, 363, -168, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -62, 243, 229, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -167, 273, 167, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -229, 303, 61, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -229, 332, -62, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -167, 963, 167, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 61, 753, -229, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 230, 813, -61, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 168, 783, -169, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 61, 1143, -229, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 230, 843, 62, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -229, 993, 61, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -167, 1053, -168, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -229, 1022, -62, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 62, 213, 229, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 230, 483, -61, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 61, 423, -229, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 230, 513, 62, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -229, 663, 61, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 168, 453, -169, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -62, 393, -228, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 61, 63, -229, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -62, 723, -228, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 230, 153, 62, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -167, 723, -168, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 169, 543, 167, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 168, 93, -169, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 230, 108, -61, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 169, 183, 167, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 62, 573, 229, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -62, 603, 229, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -167, 633, 167, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, -229, 692, -62, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_TEAR, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_CLOUD, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_SMOKE, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_ONO, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_KOZ, 0, 0 },
    { 0, 0, 0, 0, ENE_NORU_MARK, 0, 0 },
    { 1, 207, 132, 0, ENE_MONOEYE, 2, 1 },
    { 1, 382, 145, 0, ENE_MONOEYE, 2, 1 },
    { 1, 570, 215, 0, ENE_MONOEYE, 2, 1 },
    { 1, 765, 258, 0, ENE_MONOEYE, 2, 1 },
    { 1, 905, 276, 0, ENE_MONOEYE, 2, 1 },
    { 1, 1136, 363, 0, ENE_MONOEYE, 2, 1 },
    { 1, 1629, 508, 0, ENE_MONOEYE, 2, 1 },
    { 1, 1153, 340, 0, ENE_MONOEYE, 2, 1 },
    { 1, 2655, 734, 0, ENE_MONOEYE, 2, 1 },
    { 1, 2986, 822, 0, ENE_MONOEYE, 2, 1 },
    { 1, 3554, 986, 0, ENE_MONOEYE, 2, 1 },
};
ConnectData stage94PlacesEne0FAParam0[] = {
    { 30.0f, 132, 1, 4130.0f, 1161.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData stage94PlacesEne0FAParam1[] = {
    { 30.0f, 76, 1, 80.0f, 40.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef stage94Places[] = {
    { 1, 30, 80, 0, ENE_CONNECT, 2, (u32) &stage94PlacesEne0FAParam0 },
    { 1, 2376, 50, 0, ENE_CONNECT, 2, (u32) &stage94PlacesEne0FAParam1 },
    { 1, 121, 74, 0, ENE_TRANSFER, 2, 0 },
    { 1, 252, 55, 0, ENE_ITEM_KOREZO_MAIN, 2, 0 },
    { 0, -703, 40, -147, ENE_BLOODW, 2, 1 },
    { 0, -95, 107, -142, ENE_BLOODW, 2, 1 },
    { 0, 829, 36, -71, ENE_BLOODW, 2, 1 },
    { 0, -96, 37, -150, ENE_BLOODW, 2, 2 },
    { 0, 571, -20, -165, ENE_BLOODW, 2, 3 },
    { 0, -62, 108, -63, ENE_BLOODW, 2, 486148 },
    { 0, -235, 55, -24, ENE_BLOODW, 2, -3053564 },
    { 0, -122, 108, -44, ENE_BLOODW, 2, -632316 },
    { 0, -675, 40, -58, ENE_BLOODW, 2, 746500 },
    { 0, -23, 69, 38, ENE_BLOODW, 2, 1002756 },
    { 0, 815, 29, 35, ENE_BLOODW, 2, 27396 },
    { 0, 568, 13, 11, ENE_BLOODW, 2, -239100 },
    { 0, -756, 45, -68, ENE_BLOODW, 2, -1615356 },
    { 0, -1200, 35, 20, ENE_NAMI_MASTER, 2, 11796484 },
    { 0, -660, 35, -18, ENE_NAMI_MASTER, 2, 10813447 },
    { 0, 300, 5, 124, ENE_NAMI_MASTER, 2, 11796486 },
    { 0, 0, 0, 0, ENE_GIM_FBOARD, 0, 0 },
    { 1, 213, 44, 0, ENE_GIM_NBOARD, 2, 0 },
    { 1, 847, 38, 0, ENE_GIM_NBOARD, 2, 0 },
    { 1, 787, 34, 0, ENE_DEADHAND, 2, 0 },
    { 1, 811, 34, 0, ENE_DEADHAND, 2, 0 },
    { 1, 946, 37, 0, ENE_DEADHAND, 2, 0 },
    { 1, 1257, 54, 0, ENE_DEADHAND, 2, 0 },
    { 1, 1845, 7, 0, ENE_DEADHAND, 2, 0 },
    { 1, 1860, 7, 0, ENE_DEADHAND, 2, 0 },
    { 1, 2151, -4, 0, ENE_DEADHAND, 2, 0 },
    { 0, 0, 0, 0, ENE_NECKBITE_TAMA_MAIN, 0, 0 },
    { 1, 580, 95, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 1, 1433, 95, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 1, 389, 59, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 719, 60, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 1778, 50, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 330, 32, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 447, 54, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 477, 63, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 507, 72, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 565, 84, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 595, 80, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 625, 77, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 655, 82, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 884, 60, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1012, 60, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1077, 37, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1107, 37, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1137, 37, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1203, 65, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1330, 60, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1360, 65, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1390, 71, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1450, 82, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1500, 84, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1530, 80, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1560, 77, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1590, 82, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1620, 82, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1660, 82, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1690, 67, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1720, 52, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1934, 29, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 2013, -2, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 2084, 27, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 2220, 7, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 2250, 16, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
};
ConnectData stage86PlacesEne0FAParam0[] = {
    { 30.0f, 133, 1, 2330.0f, 30.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData stage86PlacesEne0FAParam1[] = {
    { 30.0f, 79, 1, 60.0f, 1.0f, 0.0f, 0.0f, 0x000A, 0, 0 },
};
WsSound2Data stage86PlacesEne05CParam[] = {
    { 6, 716, 50, 150, 0, 100, -200 },
};
WsSound2Data stage86PlacesEne05BParam[] = {
    { 6, 716, 100, 230, 0, 100, -200 },
};
EneDef stage86Places[] = {
    { 1, 12, 60, 0, ENE_CONNECT, 2, (u32) &stage86PlacesEne0FAParam0 },
    { 1, 3090, 115, 0, ENE_CONNECT, 2, (u32) &stage86PlacesEne0FAParam1 },
    { 1, 105, 40, 0, ENE_TRANSFER, 2, 0 },
    { 1, 2167, 195, 0, ENE_ETSK_GOLDNEKO_MAIN_FLAGS_0, 2, 0 },
    { 1, 1516, 95, 0, ENE_ITEM_KOREZO_MAIN, 2, 0 },
    { 0, 0, 100, 0, ENE_WS_MASTER2, 3, (u32) &stage86PlacesEne05CParam },
    { 0, 0, 84, 0, ENE_WS_SOUND2, 3, (u32) &stage86PlacesEne05CParam },
    { 0, 335, 62, 0, ENE_WS_SOUND2, 3, (u32) &stage86PlacesEne05CParam },
    { 0, 722, 155, 0, ENE_WS_SOUND2, 3, (u32) &stage86PlacesEne05BParam },
    { 0, 1114, 86, 0, ENE_WS_SOUND2, 3, (u32) &stage86PlacesEne05CParam },
    { 0, 1668, 195, 414, ENE_WS_SOUND2, 3, (u32) &stage86PlacesEne05BParam },
    { 0, 2029, 142, 708, ENE_WS_SOUND2, 3, (u32) &stage86PlacesEne05CParam },
    { 0, 2184, 121, 872, ENE_WS_SOUND2, 3, (u32) &stage86PlacesEne05CParam },
    { 0, 0, 75, 0, ENE_KAMA_HOTWATER, 2, 2 },
    { 0, 340, 38, 0, ENE_KAMA_HOTWATER, 2, 2 },
    { 0, 1120, 75, 0, ENE_KAMA_HOTWATER, 2, 2 },
    { 0, 2028, 135, 710, ENE_KAMA_HOTWATER, 2, -8388606 },
    { 0, 2188, 103, 873, ENE_KAMA_HOTWATER, 2, -8388606 },
    { 0, 720, 138, 30, ENE_KAMA_HOTWATER, 2, 3 },
    { 0, 1682, 152, 398, ENE_KAMA_HOTWATER, 2, -8388605 },
    { 0, 0, 0, 0, ENE_KAMA_BOARD, 2, 0 },
    { 0, 655, 178, 0, ENE_KAMA_BOARD_ROW, 2, 0 },
    { 0, 804, 178, 0, ENE_KAMA_BOARD_ROW, 2, 0 },
    { 0, 1621, 192, 368, ENE_KAMA_BOARD_ROW, 2, 898 },
    { 0, 1726, 192, 473, ENE_KAMA_BOARD_ROW, 2, 898 },
    { 0, 931, 162, 0, ENE_KMACE, 2, 0 },
    { 0, 994, 141, 0, ENE_KMACE, 2, 0 },
    { 0, 1704, 226, 375, ENE_KMACE, 2, 58851328 },
    { 0, 2006, 153, 685, ENE_KMACE, 2, 58851328 },
    { 0, 2053, 169, 733, ENE_KMACE, 2, 1 },
    { 0, 2097, 147, 780, ENE_KMACE, 2, 58851328 },
    { 0, 478, 120, 0, ENE_KAMA, 2, 50 },
    { 0, 1304, 120, 38, ENE_KAMA, 2, 60817438 },
    { 0, 1364, 150, 80, ENE_KAMA, 2, 59507486 },
    { 0, 1423, 180, 130, ENE_KAMA, 2, 59048478 },
    { 0, 1476, 210, 185, ENE_KAMA, 2, 57147678 },
    { 0, 1937, 188, 617, ENE_KAMA, 2, 57933854 },
    { 0, 0, 166, -20, ENE_KAMA_SASIYU, 2, 0 },
    { 0, 340, 151, -15, ENE_KAMA_SASIYU, 2, 0 },
    { 0, 1634, 292, 354, ENE_KAMA_SASIYU, 2, -128 },
    { 0, 1727, 292, 446, ENE_KAMA_SASIYU, 2, -128 },
    { 0, 2197, 211, 861, ENE_KAMA_SASIYU, 2, -128 },
    { 0, 746, 323, -201, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 82, 194, -201, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 517, 193, -201, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 1568, 193, -90, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 445, 67, -201, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 1423, 194, -201, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 1310, 67, -201, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 1703, 323, 38, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 2047, 194, 391, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 2468, 194, 808, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 2202, 194, 543, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_TEAR, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_CLOUD, 0, 0 },
    { 0, 0, 0, 0, ENE_MONOEYE_SMOKE, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_ONO, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA, 0, 0 },
    { 0, 0, 0, 0, ENE_SENSYA_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_MOUJA_KOZ, 0, 0 },
    { 0, 0, 0, 0, ENE_NORU_MARK, 0, 0 },
    { 0, 0, 0, 0, ENE_FIREBALL, 0, 0 },
    { 0, 0, 0, 0, ENE_NECKBITE_TAMA_MAIN, 0, 0 },
    { 1, 470, 95, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 1, 610, 80, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 1, 295, 113, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 386, 113, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 554, 110, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 748, 110, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 773, 165, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 967, 200, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 1040, 200, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 1200, 200, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 1485, 113, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 1677, 150, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 1755, 170, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 2045, 210, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 2215, 210, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 2290, 210, 0, ENE_HELLBAT, 2, 1280 },
    { 2, 439, 265, 0, ENE_HELLBAT, 2, 1280 },
    { 2, 541, 251, 0, ENE_HELLBAT, 2, 1280 },
    { 2, 653, 265, 0, ENE_HELLBAT, 2, 1280 },
    { 2, 750, 240, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 2485, 195, 0, ENE_HELLBAT, 2, 1280 },
    { 1, 425, 125, 0, ENE_SABURO, 2, 0 },
    { 1, 552, 125, 0, ENE_SABURO, 2, 0 },
    { 1, 668, 93, 0, ENE_SABURO, 2, 0 },
    { 1, 770, 116, 0, ENE_SABURO, 2, 0 },
    { 1, 842, 216, 0, ENE_MONOEYE, 2, 1 },
    { 1, 987, 205, 0, ENE_SABURO, 2, 0 },
    { 1, 1082, 205, 0, ENE_SABURO, 2, 0 },
    { 1, 1192, 205, 0, ENE_SABURO, 2, 0 },
    { 1, 1418, 165, 0, ENE_MONOEYE, 2, 1 },
    { 1, 1605, 130, 0, ENE_SABURO, 2, 0 },
    { 1, 1825, 210, 0, ENE_SABURO, 2, 0 },
    { 2, 540, 270, 0, ENE_SABURO, 2, 1 },
    { 2, 650, 270, 0, ENE_SABURO, 2, 1 },
    { 1, 2450, 240, 0, ENE_SABURO, 2, 1 },
    { 1, 2680, 216, 0, ENE_MONOEYE, 2, 1 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 1, 520, 106, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 640, 72, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 722, 82, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 886, 182, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1053, 182, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1220, 182, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1296, 157, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1372, 132, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1452, 107, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1053, 182, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 2444, 197, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 2777, 147, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 2, 557, 281, 0, ENE_ITEM_OOIRI_MAIN, 2, 427 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};

void stage76Build(void) {
    stageBuildBasic(0x4C, 1, 100.0f, 0.0f, 0.0f, (u32) stage76Places, 0x3E);
    if (gGameWork.tunePart <= 0) {
        soundPlayAfterOne(0xA8);
        gGameWork.tunePart = 1;
    }
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x64);
    camAiAddLimit(1, 1, 0xB54);
    gTimeOfDay = 2 << 16;
    camSunSetClockStop(1);
    gGameState.fade = 0;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
    gGameState.fadeStart = 0;
    gGameState.fadeTo[0] = 0;
    gGameState.fadeTo[1] = 0;
    gGameState.fadeTo[2] = 0;
    gGameState.fadeEnd = 0;
}

void stage77Build(void) {
    stageBuildBasic(0x4D, 1, 110.0f, 5.0f, 0.0f, (u32) stage77Places, 0x67);
    if (gGameWork.tunePart <= 0) {
        soundPlayAfterOne(0xA8);
        gGameWork.tunePart = 1;
    }
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x87);
    camAiAddLimit(1, 1, 0xF28);
    geoSetViewBox(0x96, 0x258, 0x12C, 0x258);
    gTimeOfDay = 2 << 16;
    camSunSetClockStop(1);
    gGameState.fade = 0;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
    gGameState.fadeStart = 0;
    gGameState.fadeTo[0] = 0;
    gGameState.fadeTo[1] = 0;
    gGameState.fadeTo[2] = 0;
    gGameState.fadeEnd = 0;
}

void stage81Build(void) {
    stageBuildBasic(0x51, 1, 140.0f, 5.0f, 0.0f, (u32) stage81Places, 0x31);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0xAA);
    camAiAddLimit(1, 1, 0x618);
    camAiAddLimit(2, 1, 0x271);
    if (gGameWork.tunePart < 2) {
        soundPlayAfterOne(0xA9);
        gGameWork.tunePart = 2;
    }
    gGameWork.time = 2 << 16;
    camSunSetClockStop(1);
    gGameState.fade = 0;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
    gGameState.fadeStart = 0;
    gGameState.fadeTo[0] = 0;
    gGameState.fadeTo[1] = 0;
    gGameState.fadeTo[2] = 0;
    gGameState.fadeEnd = 0;
}

void stage85Build(void) {
    stageBuildBasic(0x55, 1, 100.0f, -50.0f, 0.0f, (u32) stage85Places, 0x2B);
    camAiSetFar(800);
    camAiAddLimit(2, -1, 0x7B);
    camAiAddLimit(1, 1, 0x479);
    if (gGameWork.tunePart < 2) {
        soundPlayAfterOne(0xA9);
        gGameWork.tunePart = 2;
    }
    gGameWork.time = 2 << 16;
    camSunSetClockStop(1);
    gGameState.fade = 0;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
    gGameState.fadeStart = 0;
    gGameState.fadeTo[0] = 0;
    gGameState.fadeTo[1] = 0;
    gGameState.fadeTo[2] = 0;
    gGameState.fadeEnd = 0;
}

void stage93Build(void) {
    stageBuildBasic(0x5D, 1, 40.0f, 40.0f, 0.0f, (u32) stage93Places, 0x8F);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x32);
    camAiAddLimit(1, 1, 0x1018);
    if (gGameWork.tunePart < 2) {
        soundPlayAfterOne(0xA9);
        gGameWork.tunePart = 2;
    }
    gGameWork.time = 2 << 16;
    camSunSetClockStop(1);
    gGameState.fade = 0;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
    gGameState.fadeStart = 0;
    gGameState.fadeTo[0] = 0;
    gGameState.fadeTo[1] = 0;
    gGameState.fadeTo[2] = 0;
    gGameState.fadeEnd = 0;
}

void stage94Build(void) {
    stageBuildBasic(0x5E, 1, 90.0f, 60.0f, 0.0f, (u32) stage94Places, 0x45);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x69);
    camAiAddLimit(1, 1, 0x906);
    if (gGameWork.tunePart < 3) {
        soundPlayAfterOne(0xAA);
        gGameWork.tunePart = 3;
    }
    gGameWork.time = 2 << 16;
    camSunSetClockStop(1);
    gGameState.fade = 0;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
    gGameState.fadeStart = 0;
    gGameState.fadeTo[0] = 0;
    gGameState.fadeTo[1] = 0;
    gGameState.fadeTo[2] = 0;
    gGameState.fadeEnd = 0;
}

void stage86Build(void) {
    stageBuildBasic(0x56, 1, 80.0f, 40.0f, 0.0f, (u32) stage86Places, 0x75);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x4B);
    camAiAddLimit(1, 1, 0xBDB);
    if (gGameWork.tunePart < 3) {
        soundPlayAfterOne(0xAA);
        gGameWork.tunePart = 3;
    }
    gGameWork.time = 2 << 16;
    camSunSetClockStop(1);
    gGameState.fade = 0;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
    gGameState.fadeStart = 0;
    gGameState.fadeTo[0] = 0;
    gGameState.fadeTo[1] = 0;
    gGameState.fadeTo[2] = 0;
    gGameState.fadeEnd = 0;
}
