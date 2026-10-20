#include "tlb.h"

/* File 249: Castle 5's ten rooms. The stage list (file 13) names each builder, in file order:
 * "CASTLE 5-1-1", "5-1-2", "5-2-1", "5-2-2-1", "5-2-2-2", "5-2-2-3", "5-3-1", "5-3-2",
 * "5-4-1", "5-4-2". Each starts its tune unless it is already playing, stops the clock, and
 * sets gGameWork.tunePart to the castle's part (1-4). */

extern ConnectData castle5Places11Ene0FAParam[];
extern ExitData castle5Places11Ene0F9Param[];
extern SeriMasterData castle5Places21Ene077Param2[];
extern SeriMasterData castle5Places21Ene077Param3[];
extern ConnectData castle5Places12Ene0FAParam0[];
extern ConnectData castle5Places12Ene0FAParam1[];
extern ConnectData castle5Places21Ene0FAParam0[];
extern ConnectData castle5Places21Ene0FAParam1[];
extern ConnectData castle5Places221Ene0FAParam0[];
extern ConnectData castle5Places221Ene0FAParam1[];
extern ConnectData castle5Places222Ene0FAParam0[];
extern ConnectData castle5Places222Ene0FAParam1[];
extern ConnectData castle5Places223Ene0FAParam0[];
extern ConnectData castle5Places223Ene0FAParam1[];
extern ConnectData castle5Places31Ene0FAParam0[];
extern ConnectData castle5Places31Ene0FAParam1[];
extern u32 castle5Places31Ene05CParam[];
extern ConnectData castle5Places32Ene0FAParam0[];
extern ConnectData castle5Places32Ene0FAParam1[];
extern ConnectData castle5Places41Ene0FAParam0[];
extern ConnectData castle5Places41Ene0FAParam1[];
extern ConnectData castle5Places42Ene0FAParam0[];
extern ConnectData castle5Places42Ene0FAParam1[];
ConnectData castle5UnusedEne0FAParam[] = {
    { 30.0f, 107, 1, 576.0f, -69.0f, 0.0f, 0.0f, 0x0008, 0, 0 },
};
ConnectData castle5Places11Ene0FAParam[] = {
    { 30.0f, 107, 1, 576.0f, -69.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ExitData castle5Places11Ene0F9Param[] = {
    { 30.0f, 4, 5, -1, 0 },
};
SeriMasterData castle5Places21Ene077Param2[] = {
    { 0x00000002, 0x00000235, 0x0000051B },
};
SeriMasterData castle5Places21Ene077Param3[] = {
    { 0x00000002, 0x00000236, 0x0000051C },
};
EneDef castle5Places11[] = {
    { 1, 170, 60, 0, ENE_EXIT_ESCAPE, 2, (u32) &castle5Places11Ene0F9Param },
    { 2, 937, 320, 0, ENE_CONNECT, 2, (u32) &castle5Places11Ene0FAParam },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_POT_FRAGMENT, 0, 0 },
    { 0, 0, 0, 0, ENE_GIM_FBOARD, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, -2443, 41, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2423, 41, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2403, 41, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2320, 45, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2305, 45, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2290, 50, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2275, 50, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2260, 55, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2245, 55, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2230, 55, 0, ENE_GIM_NBOARD, 2, 256 },
    { 1, 590, 130, 0, ENE_BURANKO, 2, 80 },
    { 1, 750, 125, 0, ENE_BURANKO, 2, 65616 },
    { 1, 833, 145, 0, ENE_BURANKO, 2, 131152 },
    { 1, 989, 140, 0, ENE_BURANKO, 2, 196688 },
    { 0, -1715, 75, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1700, 80, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1685, 85, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1670, 90, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1655, 95, 0, ENE_GIM_NBOARD, 2, 256 },
    { 1, 1160, 165, 0, ENE_BURANKO, 2, 80 },
    { 0, -1545, 100, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1530, 100, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1515, 105, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1500, 105, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1485, 110, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1470, 110, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1455, 115, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1440, 115, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1425, 110, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1410, 110, 0, ENE_GIM_NBOARD, 2, 256 },
    { 1, 1413, 160, 0, ENE_BURANKO, 2, 80 },
    { 1, 1495, 157, 0, ENE_BURANKO, 2, 65616 },
    { 0, -1010, 160, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -990, 170, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -970, 180, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -950, 190, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -930, 210, 0, ENE_GIM_NBOARD, 2, 256 },
    { 0, -910, 220, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -967, 231, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -985, 242, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1001, 258, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1015, 271, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1026, 286, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1035, 301, 0, ENE_GIM_NBOARD, 2, 256 },
    { 0, -1005, 315, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -888, 230, 0, ENE_GIM_FBOARD, 2, 256 },
    { 2, 429, 393, 0, ENE_BURANKO, 2, 80 },
    { 2, 521, 389, 0, ENE_BURANKO, 2, 65616 },
    { 0, -755, 320, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -725, 310, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -695, 320, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -665, 310, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -635, 310, 0, ENE_GIM_NBOARD, 2, 256 },
    { 2, 768, 373, 0, ENE_BURANKO, 2, 80 },
    { 0, -515, 310, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -491, 316, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1010, 130, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -990, 130, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -970, 130, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -950, 130, 0, ENE_GIM_FBOARD, 2, 256 },
    { 1, 1877, 129, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1909, 125, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1945, 119, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1986, 113, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1639, 356, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1663, 352, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1693, 346, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_HAMAG_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SIROBZ, 0, 0 },
    { 1, 675, 59, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 917, 83, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1835, 138, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1897, 232, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 2, 360, 347, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 2, 917, 352, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1550, 50, 0, ENE_HAMAG_J, 2, 2 },
    { 1, 2010, 50, 0, ENE_HAMAG_J, 2, 2 },
    { 1, 530, 90, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 840, 100, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1200, 120, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1250, 130, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1500, 120, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1730, 190, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1694, 390, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1860, 160, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1800, 280, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 2, 545, 350, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 2, 745, 350, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
};
ConnectData castle5Places12Ene0FAParam0[] = {
    { 30.0f, 106, 2, 886.0f, 320.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData castle5Places12Ene0FAParam1[] = {
    { 30.0f, 134, 1, 126.0f, 15.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef castle5Places12[] = {
    { 1, 514, -58, 0, ENE_CONNECT, 2, (u32) &castle5Places12Ene0FAParam0 },
    { 1, 3420, -69, 0, ENE_CONNECT, 2, (u32) &castle5Places12Ene0FAParam1 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 1, 3310, -49, 0, ENE_TRANSFER, 2, 0 },
    { 0, -2530, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2515, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2500, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2485, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2470, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2455, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2440, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2425, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2410, -67, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2395, -67, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2380, -67, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2330, -50, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2315, -50, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2300, -50, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2285, -50, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2270, -50, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2255, -50, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2285, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2270, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2255, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2240, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2225, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2210, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2195, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2180, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2130, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2115, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2100, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2085, -72, 0, ENE_GIM_NBOARD, 2, 256 },
    { 0, -2060, -100, 0, ENE_GIM_FIREPOINT, 2, 0 },
    { 0, -2040, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2025, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -2010, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1995, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1980, -72, 0, ENE_GIM_NBOARD, 2, 256 },
    { 0, -1940, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1925, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1910, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1895, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1880, -70, 0, ENE_GIM_NBOARD, 2, 256 },
    { 0, -1860, -100, 0, ENE_GIM_FIREPOINT, 2, 0 },
    { 0, -1840, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1825, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1810, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1795, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1780, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1670, -100, 0, ENE_GIM_FIREPOINT, 2, 0 },
    { 0, -1633, -100, 0, ENE_GIM_FIREPOINT, 2, 0 },
    { 0, -1510, -60, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1480, -50, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1425, -40, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1405, -60, 0, ENE_GIM_FIREPOINT, 2, 0 },
    { 0, -1385, -30, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1315, -30, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1295, -60, 0, ENE_GIM_FIREPOINT, 2, 0 },
    { 0, -1280, -45, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1220, -50, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1190, -50, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1040, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -1010, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -980, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -950, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -920, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -890, -72, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -840, -75, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -820, -75, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -800, -75, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -780, -75, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -760, -75, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -740, -75, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -720, -75, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -700, -75, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -680, -75, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -640, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -620, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -600, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -580, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -560, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -540, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -520, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -500, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -480, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -460, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -440, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -420, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -400, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -380, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -360, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -340, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -320, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -300, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -280, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -260, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -240, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -200, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -180, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -160, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -120, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -100, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 0, -80, -70, 0, ENE_GIM_FBOARD, 2, 256 },
    { 1, 1121, -65, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 1549, -42, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1585, -52, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1622, -52, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1657, -42, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1692, -42, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1820, -52, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1922, -27, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 2021, -52, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 2410, -65, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 3211, -63, 0, ENE_ITEM_OOIRI_MAIN, 2, 420 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_HAMAG_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SIROBZ, 0, 0 },
    { 1, 673, -69, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 720, -65, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 785, -50, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 850, -45, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 920, -63, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 975, -30, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1075, -45, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1180, -50, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1250, -50, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1310, -45, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1370, -50, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1450, -45, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1720, -63, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2122, -63, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2168, -67, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 2211, -63, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1815, -30, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 2025, -30, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 2275, -45, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 2343, -45, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 2484, -50, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 2545, -45, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 2630, -50, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 2715, -45, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 2800, -50, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 2925, -45, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 3035, -50, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 3130, -45, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1603, -110, 0, ENE_HAMAG_J, 2, 2 },
    { 1, 1638, -110, 0, ENE_HAMAG_J, 2, 2 },
    { 1, 2063, -110, 0, ENE_HAMAG_J, 2, 2 },
};
ConnectData castle5Places21Ene0FAParam0[] = {
    { 30.0f, 134, 1, 283.0f, 15.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData castle5Places21Ene0FAParam1[] = {
    { 30.0f, 109, 1, 204.0f, 4.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef castle5Places21[] = {
    { 1, 169, 10, 0, ENE_CONNECT, 2, (u32) &castle5Places21Ene0FAParam0 },
    { 1, 3087, 210, 0, ENE_CONNECT, 2, (u32) &castle5Places21Ene0FAParam1 },
    { 0, 0, 100, 0, ENE_SERI_MASTER, 3, (u32) &castle5Places21Ene077Param2 },
    { 0, 0, 100, 0, ENE_SERI_MASTER, 3, (u32) &castle5Places21Ene077Param3 },
    { 0, 0, 0, 0, ENE_SERIO, 0, 0 },
    { 0, -2550, 5, 0, ENE_SERIA, 2, 15361 },
    { 0, -2475, 5, 0, ENE_SERIA, 2, 15360 },
    { 0, -2400, 5, 0, ENE_SERIA, 2, 15361 },
    { 0, -2300, 55, 0, ENE_RYUTOGE, 2, 0 },
    { 0, -2270, 15, 0, ENE_SERIA, 2, 15360 },
    { 0, -2240, 15, 0, ENE_SERIA, 2, 15361 },
    { 0, -2210, 15, 0, ENE_SERIA, 2, 15360 },
    { 0, -2180, 15, 0, ENE_SERIA, 2, 15361 },
    { 0, -2150, 55, 0, ENE_RYUTOGE, 2, 0 },
    { 0, -2120, 15, 0, ENE_SERIA, 2, 15360 },
    { 0, -2090, 15, 0, ENE_SERIA, 2, 15361 },
    { 0, -2060, 15, 0, ENE_SERIA, 2, 15360 },
    { 0, -2030, 15, 0, ENE_SERIA, 2, 15361 },
    { 0, -2000, 55, 0, ENE_RYUTOGE, 2, 0 },
    { 0, -1970, 15, 0, ENE_SERIA, 2, 15360 },
    { 0, -1940, 15, 0, ENE_SERIA, 2, 15361 },
    { 0, -1910, 15, 0, ENE_SERIA, 2, 15360 },
    { 0, -1880, 15, 0, ENE_SERIA, 2, 15361 },
    { 0, -1780, 15, 0, ENE_SERIA, 2, 15488 },
    { 0, -1750, 55, 0, ENE_RYUTOGE, 2, 0 },
    { 0, -1720, 15, 0, ENE_SERIA, 2, 15489 },
    { 0, -1690, 55, 0, ENE_RYUTOGE, 2, 0 },
    { 0, -1660, 15, 0, ENE_SERIA, 2, 15488 },
    { 0, -1630, 55, 0, ENE_RYUTOGE, 2, 0 },
    { 0, -1600, 15, 0, ENE_SERIA, 2, 15489 },
    { 0, -1570, 55, 0, ENE_RYUTOGE, 2, 0 },
    { 0, -1540, 15, 0, ENE_SERIA, 2, 15488 },
    { 0, -1462, 15, 0, ENE_SERIA, 2, 15489 },
    { 0, -1395, 15, 0, ENE_SERIA, 2, 15488 },
    { 0, -1335, 15, 0, ENE_SERIA, 2, 15489 },
    { 0, -1275, 5, 0, ENE_SERIA, 2, 15488 },
    { 0, -1225, 5, 0, ENE_SERIA, 2, 15489 },
    { 0, -1160, 15, 0, ENE_SERIA, 2, 15488 },
    { 0, -1130, 15, 0, ENE_SERIA, 2, 15489 },
    { 0, -1100, 15, 0, ENE_SERIA, 2, 15488 },
    { 0, -805, 55, 0, ENE_SERIA, 2, 15361 },
    { 0, -725, 65, 0, ENE_SERIA, 2, 15360 },
    { 0, -680, 120, 0, ENE_RYUTOGE, 2, 1 },
    { 0, -650, 130, 0, ENE_RYUTOGE, 2, 1 },
    { 0, -620, 140, 0, ENE_RYUTOGE, 2, 1 },
    { 0, -590, 150, 0, ENE_RYUTOGE, 2, 1 },
    { 0, -560, 160, 0, ENE_RYUTOGE, 2, 1 },
    { 0, -530, 170, 0, ENE_RYUTOGE, 2, 1 },
    { 0, -500, 180, 0, ENE_RYUTOGE, 2, 1 },
    { 0, -470, 190, 0, ENE_RYUTOGE, 2, 1 },
    { 0, -395, 100, 0, ENE_SERIA, 2, 15360 },
    { 0, -340, 200, 0, ENE_RYUTOGE, 2, 1 },
    { 0, -310, 220, 0, ENE_RYUTOGE, 2, 1 },
    { 0, -280, 240, 0, ENE_RYUTOGE, 2, 1 },
    { 0, -250, 220, 0, ENE_RYUTOGE, 2, 1 },
    { 0, -220, 200, 0, ENE_RYUTOGE, 2, 1 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_HAMAG_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SIROBZ, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_HAITA_TAMA1, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_HAITA_TAMA2, 0, 0 },
    { 1, 361, 25, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 394, 38, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 452, 52, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 745, 52, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 2138, 52, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 2138, 52, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 2200, 73, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 2249, 92, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 2175, 80, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 2223, 100, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 2452, 162, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 2486, 205, 0, ENE_SIRO_J_HOURS_ANY, 2, 0 },
    { 1, 2530, 198, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 2591, 235, 0, ENE_SIRO_J_HOURS_ANY, 2, 0 },
    { 1, 2684, 192, 0, ENE_ETSK_HAITA_MAIN, 2, 0 },
    { 1, 2736, 225, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 2775, 278, 0, ENE_SIRO_J_HOURS_ANY, 2, 0 },
    { 1, 2833, 244, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 2946, 192, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 1 },
};
ConnectData castle5Places221Ene0FAParam0[] = {
    { 30.0f, 108, 1, 3025.0f, 190.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData castle5Places221Ene0FAParam1[] = {
    { 30.0f, 113, 1, 822.0f, 47.0f, 0.0f, 0.0f, 0x0000, 0, 2 },
};
EneDef castle5Places221[] = {
    { 1, 145, 4, 0, ENE_CONNECT, 2, (u32) &castle5Places221Ene0FAParam0 },
    { 1, 668, -60, 0, ENE_CONNECT, 2, (u32) &castle5Places221Ene0FAParam1 },
    { 1, 748, 24, 0, ENE_TRANSFER, 2, 0 },
    { 1, 606, -60, 0, ENE_CONNECT, 2, (u32) &castle5Places221Ene0FAParam1 },
    { 1, 715, -60, 0, ENE_CONNECT, 2, (u32) &castle5Places221Ene0FAParam1 },
    { 1, 745, -60, 0, ENE_CONNECT, 2, (u32) &castle5Places221Ene0FAParam1 },
    { 1, 767, -60, 0, ENE_CONNECT, 2, (u32) &castle5Places221Ene0FAParam1 },
    { 0, -2305, 3, -435, ENE_OBJ22_WATER_FLAGS_3, 2, 3 },
    { 1, 280, 40, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 330, 40, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 380, 40, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 430, 40, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 480, 40, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 530, 40, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 300, 6, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 400, 6, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 500, 6, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 600, 6, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
};
ConnectData castle5Places222Ene0FAParam0[] = {
    { 30.0f, 109, 1, 668.0f, -20.0f, 0.0f, 0.0f, 0x0000, 0, 4 },
};
ConnectData castle5Places222Ene0FAParam1[] = {
    { 30.0f, 114, 1, 547.0f, 37.0f, 0.0f, 0.0f, 0x0000, 0, 4 },
};
EneDef castle5Places222[] = {
    { 1, 826, 127, 0, ENE_CONNECT, 2, (u32) &castle5Places222Ene0FAParam0 },
    { 1, 2679, 198, 0, ENE_CONNECT, 2, (u32) &castle5Places222Ene0FAParam1 },
    { 1, 771, -65, 0, ENE_ETSK_GOLDNEKO_MAIN_FLAGS_0, 2, 0 },
    { 0, 0, 10, 0, ENE_AQUA, 2, 0 },
    { 0, 0, 207, 0, ENE_OBJ22_WATER_FLAGS_3, 2, 3 },
    { 0, -750, 207, 0, ENE_OBJ22_WATER_FLAGS_3, 2, 3 },
    { 0, -1500, 207, 0, ENE_OBJ22_WATER_FLAGS_3, 2, 3 },
    { 0, -2500, 207, 0, ENE_OBJ22_WATER_FLAGS_3, 2, 3 },
    { 1, 635, 30, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 635, -50, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 715, 30, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 715, -50, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 900, 30, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 900, -50, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 1000, 30, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 1025, -50, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 1100, 30, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 1070, -50, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 1230, 30, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 1230, -50, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 1395, 60, 0, ENE_TOGEITA_SIDE, 2, 0 },
    { 1, 1465, 10, 0, ENE_TOGEITA_SIDE, 2, 0 },
    { 1, 1600, 25, 0, ENE_TOGEITA_SIDE, 2, 0 },
    { 1, 1705, 0, 0, ENE_TOGEITA_SIDE, 2, 0 },
    { 1, 2010, 90, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 2030, 15, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 2100, 90, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 2080, 15, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 2270, 90, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 2270, 15, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 2510, 60, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 2550, 60, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 2795, 70, 0, ENE_TOGEITA_SIDE, 2, 0 },
    { 1, 2885, 50, 0, ENE_TOGEITA_SIDE, 2, 0 },
    { 1, 2815, 150, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 1, 2930, 150, 0, ENE_TOGEITA_UP_DOWN, 2, 0 },
    { 0, 0, 0, 0, ENE_ELEFISH_LIGHTNING, 0, 0 },
    { 0, 0, 0, 0, ENE_TAKO_SUMI_MAIN, 0, 0 },
    { 1, 540, 25, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 540, -60, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 960, -55, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1080, -50, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1635, 10, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1850, 60, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 2050, 5, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 2980, 135, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 2980, 60, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 9 },
    { 1, 1367, -42, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 1570, 80, 0, ENE_ETSK_JELLY_MAIN, 2, 1 },
    { 1, 1815, 12, 0, ENE_ETSK_JELLY_MAIN, 2, 2 },
    { 1, 2200, 22, 0, ENE_ETSK_JELLY_MAIN, 2, 3 },
    { 1, 2641, 78, 0, ENE_ETSK_JELLY_MAIN, 2, 0 },
    { 1, 2714, 57, 0, ENE_ETSK_JELLY_MAIN, 2, 1 },
    { 1, 600, 40, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 600, -60, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 700, 20, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 660, -40, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 932, 22, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1037, 40, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 960, -60, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1060, -40, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1147, 20, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1241, 40, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 2200, 85, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 2275, -5, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1610, 20, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1750, -5, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 2060, 85, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 2550, 75, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 2550, 140, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 2937, 150, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 2831, 130, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 2855, 50, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 2966, 70, 0, ENE_ETSK_ELEFISH_MAIN, 2, 2 },
    { 1, 1400, 50, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1431, -3, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1606, 5, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1742, 5, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 2060, 5, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1984, 83, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 2097, 80, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 2314, 11, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 2367, 82, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 2498, 60, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 2533, 136, 0, ENE_ETSK_ELEFISH_MAIN, 2, 3 },
    { 1, 1280, -35, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 1344, 15, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 1526, 54, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 1753, 54, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 1883, 23, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 1957, 70, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 2335, -20, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 1, 2403, 67, 0, ENE_ETSK_TAKO_MAIN, 2, 1 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 1, 574, -63, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 2343, -7, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 1805, 107, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 2982, 45, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
};
ConnectData castle5Places223Ene0FAParam0[] = {
    { 20.0f, 113, 1, 2680.0f, 140.0f, 0.0f, 0.0f, 0x0000, 0, 2 },
};
ConnectData castle5Places223Ene0FAParam1[] = {
    { 30.0f, 135, 1, 126.0f, 15.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef castle5Places223[] = {
    { 1, 502, 9, 0, ENE_CONNECT, 2, (u32) &castle5Places223Ene0FAParam0 },
    { 1, 827, 76, 0, ENE_CONNECT, 2, (u32) &castle5Places223Ene0FAParam1 },
    { 0, 0, 53, 0, ENE_OBJ22_WATER_FLAGS_3, 2, 3 },
    { 1, 705, 106, 0, ENE_TRANSFER, 2, 0 },
    { 1, 230, 80, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 1, 280, 80, 0, ENE_ETSK_YADO_MAIN, 2, 1 },
    { 1, 350, 80, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 1, 620, 80, 0, ENE_ETSK_YADO_MAIN, 2, 1 },
};
ConnectData castle5Places31Ene0FAParam0[] = {
    { 30.0f, 135, 1, 288.0f, 15.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData castle5Places31Ene0FAParam1[] = {
    { 30.0f, 116, 1, 116.0f, -15.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
u32 castle5Places31Ene05CParam[] = { 0x00000004, 0x000002CC, 0x0000003C, 0x00000078, 0x00000000 };
EneDef castle5Places31[] = {
    { 1, 21, 40, 0, ENE_CONNECT, 2, (u32) &castle5Places31Ene0FAParam0 },
    { 1, 2112, 188, 0, ENE_CONNECT, 2, (u32) &castle5Places31Ene0FAParam1 },
    { 0, 82, 60, -195, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 352, 60, -195, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 697, 60, -195, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 1018, 60, -133, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 1352, 60, 62, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 1014, 186, -127, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 1354, 186, 61, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 1514, 186, 212, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 1606, 186, 379, ENE_TOMOSIBI_FLAGS_2, 2, 0 },
    { 0, 0, 100, 0, ENE_WS_MASTER2, 3, (u32) &castle5Places31Ene05CParam },
    { 0, 440, 35, 0, ENE_WS_SOUND2, 3, (u32) &castle5Places31Ene05CParam },
    { 0, 1095, 120, 165, ENE_WS_SOUND2, 3, (u32) &castle5Places31Ene05CParam },
    { 0, -107, 70, 2, ENE_KAMA, 2, 852274 },
    { 0, 25, 70, 2, ENE_KAMA, 2, 50 },
    { 0, 57, 58, 0, ENE_KMACE, 2, 1 },
    { 0, 143, 50, 2, ENE_KAMA, 2, 50 },
    { 0, 268, 50, 10, ENE_KAMA, 2, 306 },
    { 0, 441, 136, -19, ENE_KAMA_SASIYU, 2, 0 },
    { 0, 443, 6, 0, ENE_KAMA_HOTWATER, 2, 2 },
    { 0, 615, 70, 0, ENE_KAMA, 2, 50 },
    { 0, 690, 120, 0, ENE_KAMA, 2, 286 },
    { 0, 930, 120, 71, ENE_KAMA, 2, 61472818 },
    { 0, 854, 122, 27, ENE_KMACE, 2, 61145088 },
    { 0, 904, 122, 55, ENE_KMACE, 2, 61931520 },
    { 0, 1180, 112, 214, ENE_KMACE, 2, 60686336 },
    { 0, 1473, 157, 384, ENE_KMACE, 2, 61407232 },
    { 0, 1236, 115, 250, ENE_KMACE, 2, 1 },
    { 0, 1300, 185, 293, ENE_KMACE, 2, 1 },
    { 0, 1100, 212, 142, ENE_KAMA_SASIYU, 2, 940 },
    { 0, 1092, 76, 162, ENE_KAMA_HOTWATER, 2, 2 },
    { 0, 1261, 130, 263, ENE_KAMA, 2, 60686386 },
    { 0, 1368, 150, 324, ENE_KAMA, 2, 60293170 },
    { 0, 1550, 180, 434, ENE_KAMA, 2, 61079602 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_HAMAG_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SIROBZ, 0, 0 },
    { 0, 0, 0, 0, ENE_BOATSKEL_HEAD_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_NECKBITE_TAMA_MAIN, 0, 0 },
    { 1, 157, 42, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 2 },
    { 1, 298, 47, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 1, 421, 86, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 1, 675, 73, -69, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 790, 37, -49, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 904, 70, -66, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 1100, 120, -54, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 1235, 130, -52, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 1447, 130, -56, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 1537, 130, -56, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 1532, 125, 0, ENE_ETSK_NECKBITE_MAIN, 2, 0 },
    { 1, 1765, 167, -63, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 1867, 167, -57, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 1954, 167, -57, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 707, 24, 0, ENE_BOATSKEL_HEAD_MAIN, 2, 0 },
    { 1, 713, 24, 0, ENE_BOATSKEL_HEAD_MAIN, 2, 0 },
    { 1, 716, 24, 13, ENE_BOATSKEL_HEAD_MAIN, 2, 0 },
    { 1, 714, 24, -13, ENE_BOATSKEL_HEAD_MAIN, 2, 0 },
    { 1, 849, 24, 0, ENE_BOATSKEL_HEAD_MAIN, 2, 0 },
    { 1, 845, 24, -1, ENE_BOATSKEL_HEAD_MAIN, 2, 0 },
    { 1, 1403, 100, 0, ENE_BOATSKEL_HEAD_MAIN, 2, 0 },
    { 1, 1405, 100, -11, ENE_BOATSKEL_HEAD_MAIN, 2, 0 },
    { 1, 754, 52, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 1, 843, 52, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 1, 1136, 127, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 1, 1172, 132, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 1, 1380, 122, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 1, 1888, 167, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
};
ConnectData castle5Places32Ene0FAParam0[] = {
    { 30.0f, 115, 1, 2063.0f, 188.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData castle5Places32Ene0FAParam1[] = {
    { 30.0f, 136, 1, 126.0f, 15.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef castle5Places32[] = {
    { 1, 68, -15, 0, ENE_CONNECT, 2, (u32) &castle5Places32Ene0FAParam0 },
    { 1, 1466, 270, 0, ENE_CONNECT, 2, (u32) &castle5Places32Ene0FAParam1 },
    { 0, -40, 10, -275, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 98, 27, -268, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 238, 35, -275, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 375, 48, -269, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 514, 67, -275, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 653, 67, -271, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 779, 69, -241, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 895, 69, -167, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 1019, 69, -102, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 1139, 68, -30, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 779, 188, -241, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 898, 188, -169, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 1019, 187, -102, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 1136, 188, -28, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 779, 308, -241, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 896, 308, -167, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 1019, 307, -102, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 1136, 308, -28, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 1260, 306, 37, ENE_TOMOSIBI_FLAGS_0, 2, 0 },
    { 0, 74, -2, -218, ENE_HMITA, 2, 0 },
    { 0, 183, 6, -220, ENE_HMITA, 2, 0 },
    { 0, 297, 30, -220, ENE_HMITA, 2, 0 },
    { 0, 581, 75, -220, ENE_HMITA, 2, 0 },
    { 0, 850, 115, -145, ENE_HMITA, 2, 946 },
    { 0, 1015, 260, -52, ENE_HMITA, 2, 938 },
    { 0, 806, 231, -165, ENE_KMACE, 2, 1 },
    { 0, 777, 108, -164, ENE_KMACE, 2, 62062592 },
    { 0, 373, 32, -220, ENE_PPRESS, 2, 25600 },
    { 0, 417, 32, -220, ENE_PPRESS, 2, 25600 },
    { 0, 507, 32, -220, ENE_PPRESS, 2, 25600 },
    { 0, 657, 100, -225, ENE_PPRESS, 2, 25600 },
    { 0, 744, 100, -200, ENE_PPRESS, 2, 63726592 },
    { 0, 947, 100, -92, ENE_PPRESS, 2, 62153728 },
    { 0, 947, 260, -92, ENE_PPRESS, 2, 61957120 },
    { 1, 280, 3, 0, ENE_DEADHAND, 2, 0 },
    { 1, 381, 11, -9, ENE_DEADHAND, 2, 0 },
    { 1, 396, 11, 5, ENE_DEADHAND, 2, 0 },
    { 1, 493, 35, -9, ENE_DEADHAND, 2, 0 },
    { 1, 504, 35, 0, ENE_DEADHAND, 2, 0 },
    { 1, 510, 35, 6, ENE_DEADHAND, 2, 0 },
    { 1, 785, 80, 7, ENE_DEADHAND, 2, 0 },
    { 1, 778, 80, 0, ENE_DEADHAND, 2, 0 },
    { 1, 785, 80, -9, ENE_DEADHAND, 2, 0 },
    { 1, 1062, 120, -4, ENE_DEADHAND, 2, 0 },
    { 1, 1078, 120, -5, ENE_DEADHAND, 2, 0 },
    { 1, 1259, 265, 0, ENE_DEADHAND, 2, 0 },
    { 1, 445, 25, 0, ENE_DEADHAND, 2, 0 },
    { 1, 557, 44, 0, ENE_DEADHAND, 2, 0 },
    { 1, 599, 60, 0, ENE_DEADHAND, 2, 0 },
    { 1, 735, 70, 0, ENE_DEADHAND, 2, 0 },
    { 1, 838, 96, 0, ENE_DEADHAND, 2, 0 },
    { 1, 882, 101, 0, ENE_DEADHAND, 2, 0 },
    { 1, 928, 107, 0, ENE_DEADHAND, 2, 0 },
    { 1, 972, 111, 0, ENE_DEADHAND, 2, 0 },
    { 1, 1121, 140, 0, ENE_DEADHAND, 2, 0 },
    { 1, 1142, 140, 0, ENE_DEADHAND, 2, 0 },
    { 1, 1162, 140, 0, ENE_DEADHAND, 2, 0 },
    { 1, 1211, 249, 0, ENE_DEADHAND, 2, 0 },
    { 1, 645, 100, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 655, 100, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 665, 100, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 675, 100, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 685, 100, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1130, 280, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1140, 280, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1150, 280, 0, ENE_ETSK_WETWOMAN_MAIN_HOURS_ANY, 2, 3 },
    { 1, 596, 62, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 734, 69, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 839, 97, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 880, 102, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 925, 107, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 971, 112, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1015, 112, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1200, 160, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1149, 180, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1094, 200, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1100, 237, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1211, 249, 0, ENE_ETSK_KONAKI_MAIN_HOURS_ANY, 2, 1 },
};
ConnectData castle5Places41Ene0FAParam0[] = {
    { 20.0f, 136, 1, 290.0f, 15.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData castle5Places41Ene0FAParam1[] = {
    { 20.0f, 118, 1, 114.0f, 54.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef castle5Places41[] = {
    { 1, 40, 81, 0, ENE_CONNECT, 2, (u32) &castle5Places41Ene0FAParam0 },
    { 1, 1898, 118, 0, ENE_CONNECT, 2, (u32) &castle5Places41Ene0FAParam1 },
    { 0, -306, 24, -12, ENE_TRAP_GUPPA, 2, 1114112 },
    { 0, 59, -7, -131, ENE_TRAP_GUPPA, 2, 7798784 },
    { 0, 179, -20, -166, ENE_TRAP_GUPPA, 2, 0 },
    { 0, 246, -36, -169, ENE_TRAP_GUPPA, 2, 0 },
    { 0, 520, -11, -189, ENE_TRAP_GUPPA, 2, 0 },
    { 0, 592, -27, -179, ENE_TRAP_GUPPA, 2, 64225280 },
    { 0, 658, -40, -159, ENE_TRAP_GUPPA, 2, 64225280 },
    { 0, 725, -32, -150, ENE_TRAP_GUPPA, 2, 131072 },
    { 0, 1089, 65, -149, ENE_TRAP_GUPPA, 2, 0 },
    { 1, 215, 200, 0, ENE_TRANSFER, 2, 0 },
    { 0, -433, 84, 0, ENE_TRAP_BPROP, 2, 8388658 },
    { 0, -370, 64, 0, ENE_TRAP_BPROP, 2, 8389150 },
    { 0, -370, 180, 0, ENE_TRAP_BPROP, 2, 8389160 },
    { 0, 20, 60, -72, ENE_TRAP_BPROP, 2, 65994782 },
    { 0, 120, 60, -142, ENE_TRAP_BPROP, 2, 10813470 },
    { 0, 868, 25, -140, ENE_TRAP_BPROP, 2, 8388638 },
    { 0, 919, 55, -140, ENE_TRAP_BPROP, 2, 8388894 },
    { 0, 970, 85, -140, ENE_TRAP_BPROP, 2, 8389150 },
    { 0, 1019, 115, -140, ENE_TRAP_BPROP, 2, 8388638 },
    { 0, 0, 0, 0, ENE_SHIBAG_IWA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_MIMI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_HAITA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_HAITA_TAMA1, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_HAITA_TAMA2, 0, 0 },
    { 1, 393, 74, 0, ENE_ETSK_SHIBAG_MAIN, 2, 0 },
    { 1, 542, 57, 0, ENE_ETSK_SHIBAG_MAIN, 2, 0 },
    { 1, 1000, 7, 0, ENE_ETSK_SHIBAG_MAIN, 2, 0 },
    { 1, 1147, 22, 0, ENE_ETSK_SHIBAG_MAIN, 2, 0 },
    { 1, 437, 71, 0, ENE_DEADHAND, 2, 0 },
    { 1, 509, 57, 0, ENE_DEADHAND, 2, 0 },
    { 1, 1040, 8, 0, ENE_DEADHAND, 2, 0 },
    { 1, 1110, 20, 0, ENE_DEADHAND, 2, 0 },
    { 1, 600, 51, 0, ENE_DEADHAND, 2, 0 },
    { 1, 620, 53, 0, ENE_DEADHAND, 2, 0 },
    { 1, 640, 55, 0, ENE_DEADHAND, 2, 0 },
    { 1, 1480, 29, 0, ENE_MIMI, 2, 1 },
    { 1, 1515, 29, 0, ENE_MIMI, 2, 1 },
    { 1, 476, 65, 0, ENE_ETSK_HAITA_MAIN, 2, 0 },
    { 1, 1073, 16, 0, ENE_ETSK_HAITA_MAIN, 2, 0 },
    { 1, 1500, 31, 0, ENE_ETSK_HAITA_MAIN, 2, 0 },
    { 1, 343, 88, 0, ENE_SABURO, 2, 0 },
    { 1, 622, 75, 0, ENE_SABURO, 2, 0 },
    { 1, 726, 75, 0, ENE_SABURO, 2, 0 },
    { 1, 783, 75, 0, ENE_SABURO, 2, 0 },
    { 1, 850, 57, 0, ENE_SABURO, 2, 0 },
    { 1, 911, 50, 0, ENE_SABURO, 2, 0 },
    { 1, 1250, 50, 0, ENE_SABURO, 2, 0 },
    { 1, 1315, 40, 0, ENE_SABURO, 2, 0 },
    { 1, 1390, 30, 0, ENE_SABURO, 2, 0 },
    { 1, 1590, 70, 0, ENE_SABURO, 2, 0 },
    { 1, 1645, 100, 0, ENE_SABURO, 2, 0 },
    { 1, 1700, 140, 0, ENE_SABURO, 2, 0 },
    { 1, 279, 271, 0, ENE_ITEM_OOIRI_MAIN, 2, 432 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};
ConnectData castle5Places42Ene0FAParam0[] = {
    { 20.0f, 117, 1, 1853.0f, 118.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData castle5Places42Ene0FAParam1[] = {
    { 30.0f, 137, 1, 125.0f, 15.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef castle5Places42[] = {
    { 1, 69, 54, 0, ENE_CONNECT, 2, (u32) &castle5Places42Ene0FAParam0 },
    { 1, 2471, 54, 0, ENE_CONNECT, 2, (u32) &castle5Places42Ene0FAParam1 },
    { 1, 2331, 34, 0, ENE_TRANSFER, 2, 0 },
    { 0, 182, -65, -259, ENE_BOZE, 2, 1 },
    { 0, -445, -100, -261, ENE_MAGMA_FLAGS_3, 2, 14 },
    { 0, 1245, -100, -131, ENE_MAGMA_FLAGS_3, 2, 14 },
    { 0, -445, -120, -261, ENE_MAGMA_FLAGS_3, 2, 14 },
    { 0, 1245, -120, -131, ENE_MAGMA_FLAGS_3, 2, 14 },
    { 0, 122, -69, 2, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, 173, -69, 2, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, 224, -69, 2, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, 274, -69, 2, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, 322, -69, 0, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, 373, -69, 0, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, 480, -69, 6, ENE_BOZE_UBRIDGE, 2, 716 },
    { 0, 520, -69, 24, ENE_BOZE_UBRIDGE, 2, 693 },
    { 0, 552, -69, 49, ENE_BOZE_UBRIDGE, 2, 648 },
    { 0, 589, -69, 83, ENE_BOZE_UBRIDGE, 2, 641 },
    { 0, 717, -69, 212, ENE_BOZE_UBRIDGE, 2, 643 },
    { 0, 751, -69, 245, ENE_BOZE_UBRIDGE, 2, 643 },
    { 0, 787, -69, 280, ENE_BOZE_UBRIDGE, 2, 642 },
    { 0, 820, -69, 313, ENE_BOZE_UBRIDGE, 2, 641 },
    { 0, 959, -69, 372, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, 1009, -69, 374, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, 1060, -69, 374, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, 1109, -69, 374, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, 1210, -69, 373, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, 1260, -69, 373, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, 1310, -69, 373, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, 1360, -69, 373, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, 1410, -69, 373, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, 1460, -69, 373, ENE_BOZE_UBRIDGE, 2, 768 },
    { 0, -262, 26, 4, ENE_TRAP_BPROP, 2, 8388638 },
    { 0, -210, 26, 2, ENE_TRAP_BPROP, 2, 8388894 },
    { 0, -154, -2, 2, ENE_TRAP_BPROP, 2, 8389150 },
    { 0, -105, -46, 2, ENE_TRAP_BPROP, 2, 8388638 },
    { 0, 426, -76, -1, ENE_TRAP_BPROP, 2, 8388638 },
    { 0, 628, -76, 123, ENE_TRAP_BPROP, 2, 8388638 },
    { 0, 676, -73, 172, ENE_TRAP_BPROP, 2, 8388894 },
    { 0, 858, -73, 343, ENE_TRAP_BPROP, 2, 8388638 },
    { 0, 911, -73, 367, ENE_TRAP_BPROP, 2, 8388894 },
    { 0, 1158, -84, 374, ENE_TRAP_BPROP, 2, 8388894 },
    { 0, 1511, -64, 372, ENE_TRAP_BPROP, 2, 8388894 },
    { 0, 1642, -30, 375, ENE_TRAP_BPROP, 2, 8388638 },
    { 0, 1693, -12, 372, ENE_TRAP_BPROP, 2, 8388894 },
    { 0, 0, 0, 0, ENE_ETSK_HAITA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_HAITA_TAMA1, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_HAITA_TAMA2, 0, 0 },
    { 1, 410, -40, 0, ENE_ETSK_HAITA_MAIN, 2, 0 },
    { 1, 490, -40, 0, ENE_ETSK_HAITA_MAIN, 2, 0 },
    { 1, 2162, -40, 0, ENE_ETSK_HAITA_MAIN, 2, 0 },
    { 1, 1243, -69, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1262, -101, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1272, -101, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1293, -69, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1310, -101, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1320, -101, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1342, -69, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1360, -101, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1370, -101, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1393, -69, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1545, -69, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1560, -101, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1570, -101, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1593, -69, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1610, -101, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1620, -101, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1644, -69, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1660, -101, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1670, -101, 0, ENE_DEADHAND, 2, 1 },
    { 1, 1694, -69, 0, ENE_DEADHAND, 2, 1 },
};

void castle5Init11(void) {
    frmDeleteKind(gFrmKindGpol);
    polInitGpolKind(150);
    stageBuildBasic(0x63, 1, 220.0f, 60.0f, 0.0f, (u32) castle5Places11, 0x61);
    camAiSetFar(600);
    camSunSetProgram(9);
    camAiAddLimit(1, -1, 0xE6);
    camAiAddLimit(2, 1, 0x370);
    if (sndGetVolume(0xAE, 0) == 0) {
        soundPlayAfterOne(0xAE);
    }
    gTimeOfDay = 12 << 16;
    camSunSetClockStop(1);
    gGameWork.tunePart = 1;
}

void castle5Init12(void) {
    frmDeleteKind(gFrmKindGpol);
    polInitGpolKind(150);
    stageBuildBasic(0x64, 1, 100.0f, 100.0f, 0.0f, (u32) castle5Places12, 0x9A);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x244);
    camAiAddLimit(1, 1, 0xD0C);
    if (sndGetVolume(0xAE, 0) == 0) {
        soundPlayAfterOne(0xAE);
    }
    gTimeOfDay = 12 << 16;
    camSunSetClockStop(1);
    gGameWork.tunePart = 1;
}

void castle5Init21(void) {
    stageBuildBasic(0x65, 1, 250.0f, 10.0f, 0.0f, (u32) castle5Places21, 0x51);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0xFA);
    camAiAddLimit(1, 1, 0xBD1);
    if (sndGetVolume(0xA5, 0) == 0) {
        soundPlayAfterOne(0xA5);
    }
    gTimeOfDay = 12 << 16;
    camSunSetClockStop(1);
    gGameWork.tunePart = 2;
}

void castle5Init221(void) {
    stageBuildBasic(0x6A, 1, 250.0f, 10.0f, 0.0f, (u32) castle5Places221, 0x12);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0xD2);
    if (sndGetVolume(0xA5, 0) == 0) {
        soundPlayAfterOne(0xA5);
    }
    gTimeOfDay = 12 << 16;
    camSunSetClockStop(1);
    gGameWork.tunePart = 2;
}

void castle5Init222(void) {
    stageBuildDive(0x6B, 1, 250.0f, 10.0f, 0.0f, (u32) castle5Places222, 0x62);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x208);
    camAiAddLimit(1, 1, 0xBB8);
    camSunSetProgram(4);
    sndSetReverb(1, 0xF);
    if (sndGetVolume(0xA7, 0) == 0) {
        soundPlayAfterOne(0xA7);
    }
    gTimeOfDay = 12 << 16;
    camSunSetClockStop(1);
    gGameWork.tunePart = 1;
}

void castle5Init223(void) {
    stageBuildBasic(0x6C, 1, 250.0f, 10.0f, 0.0f, (u32) castle5Places223, 8);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0xE6);
    camAiAddLimit(1, 1, 0x302);
    if (sndGetVolume(0xA5, 0) == 0) {
        soundPlayAfterOne(0xA5);
    }
    gTimeOfDay = 12 << 16;
    camSunSetClockStop(1);
    gGameWork.tunePart = 2;
}

void castle5Init31(void) {
    stageBuildBasic(0x66, 1, 80.0f, 40.0f, 0.0f, (u32) castle5Places31, 0x45);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x5A);
    camAiAddLimit(1, 1, 0x80C);
    if (sndGetVolume(0xAF, 0) == 0) {
        soundPlayAfterOne(0xAF);
    }
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
    gGameWork.tunePart = 3;
}

void castle5Init32(void) {
    stageBuildBasic(0x67, 1, 135.0f, -15.0f, 0.0f, (u32) castle5Places32, 0x50);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x82);
    camAiAddLimit(1, 1, 0x587);
    if (sndGetVolume(0xAF, 0) == 0) {
        soundPlayAfterOne(0xAF);
    }
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
    gGameWork.tunePart = 3;
}

void castle5Init41(void) {
    stageBuildBasic(0x68, 1, 90.0f, 81.0f, 0.0f, (u32) castle5Places41, 0x3A);
    camAiSetFar(1200);
    camAiAddLimit(1, -1, 0x5A);
    camAiAddLimit(1, 1, 0x73A);
    if (sndGetVolume(0xB0, 0) == 0) {
        soundPlayAfterOne(0xB0);
    }
    gTimeOfDay = 12 << 16;
    camSunSetClockStop(1);
    camSunSetProgram(9);
    gGameWork.tunePart = 4;
}

void castle5Init42(void) {
    geoSetViewBox(400, 600, 300, 600);
    stageBuildBasic(0x69, 1, 110.0f, 54.0f, 0.0f, (u32) castle5Places42, 0x47);
    camAiSetFar(1000);
    camAiAddLimit(1, -1, 0x64);
    camAiAddLimit(1, 1, 0x96A);
    if (sndGetVolume(0xB0, 0) == 0) {
        soundPlayAfterOne(0xB0);
    }
    gTimeOfDay = 12 << 16;
    camSunSetClockStop(1);
    camSunSetProgram(9);
    gGameWork.tunePart = 4;
}
