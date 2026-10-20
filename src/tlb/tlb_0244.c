#include "tlb.h"

extern ConnectData castle41PlacesEne0FAParam[];
extern ExitData castle41PlacesEne0F9Param[];
extern ConnectData castle42PlacesEne0FAParam0[];
extern ConnectData castle42PlacesEne0FAParam1[];
extern ConnectData castle43PlacesEne0FAParam0[];
extern ConnectData castle43PlacesEne0FAParam1[];
extern ConnectData castle43PlacesEne0FAParam2[];
extern ConnectData castle44PlacesEne0FAParam0[];
extern ConnectData castle44PlacesEne0FAParam1[];
extern ConnectData castle45PlacesEne0FAParam0[];
extern ConnectData castle45PlacesEne0FAParam1[];
extern ConnectData castle46Places2AEne0FAParam[];
extern ConnectData castle46PlacesEne0FAParam[];
extern ConnectData castle46Places2BEne0FAParam[];
extern ConnectData castle47PlacesEne0FAParam0[];
extern ConnectData castle47PlacesEne0FAParam1[];
extern ConnectData castle48PlacesEne0FAParam[];
ConnectData castle41PlacesEne0FAParam[] = {
    { 30.0f, 88, 1, 80.0f, 50.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ExitData castle41PlacesEne0F9Param[] = {
    { 60.0f, 3, 5, -1, 0 },
};
EneDef castle41Places[] = {
    { 1, 346, 50, 0, ENE_EXIT_ESCAPE, 2, (u32) &castle41PlacesEne0F9Param },
    { 1, 1640, -40, 0, ENE_CONNECT, 2, (u32) &castle41PlacesEne0FAParam },
    { 0, 0, 0, 0, ENE_POT_FRAGMENT, 0, 0 },
    { 1, 1361, -90, 0, ENE_TRANSFER, 2, 0 },
    { 1, 415, 65, 0, ENE_ITEM_OOIRI_MAIN, 2, 429 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 1, 85, -46, 0, ENE_POT, 2, 0 },
    { 1, 125, -46, 0, ENE_POT, 2, 0 },
    { 1, 165, -46, 0, ENE_POT, 2, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_WANYU_FIRE_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_BIWA_FIRE, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_BIWA_KUMO, 0, 0 },
    { 1, 620, -20, -5, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 660, -20, -5, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 720, 5, 0, ENE_ETSK_BIWA_MAIN, 2, 1 },
    { 1, 740, -48, 50, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 760, -48, 50, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 780, -48, 50, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 780, -48, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 810, -48, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 880, -48, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 960, -50, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 970, -23, 0, ENE_ETSK_BIWA_MAIN, 2, 1 },
    { 1, 1080, -34, 0, ENE_ETSK_WANYU_MAIN, 2, 1 },
    { 1, 1110, -74, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1185, -70, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 5 },
    { 1, 1260, -55, 0, ENE_ETSK_WANYU_MAIN, 2, 1 },
    { 1, 1270, -90, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1380, -100, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1430, -90, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1475, -80, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1495, -70, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1575, -68, -35, ENE_ETSK_YOROI_MAIN, 2, 4 },
    { 1, 1655, -68, -35, ENE_ETSK_YOROI_MAIN, 2, 4 },
    { 1, 1685, -68, -35, ENE_ETSK_YOROI_MAIN, 2, 4 },
};
ConnectData castle42PlacesEne0FAParam0[] = {
    { 30.0f, 87, 1, 1575.0f, -65.0f, 0.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData castle42PlacesEne0FAParam1[] = {
    { 30.0f, 89, 1, 195.0f, -5.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef castle42Places[] = {
    { 1, 30, 75, 0, ENE_CONNECT, 2, (u32) &castle42PlacesEne0FAParam0 },
    { 1, 1810, 75, 0, ENE_CONNECT, 2, (u32) &castle42PlacesEne0FAParam1 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 1, 160, 38, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 205, 38, 0, ENE_EYEFROG_MAIN, 2, 3 },
    { 1, 220, 51, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 250, 51, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 295, 51, 0, ENE_EYEFROG_MAIN, 2, 3 },
    { 1, 390, 53, 0, ENE_ETSK_MAJO_MAIN, 2, 1 },
    { 1, 360, 40, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 380, 40, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 400, 30, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 420, 30, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 440, 30, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 460, 20, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 480, 20, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 500, 20, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 520, 10, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 540, 10, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 1660, 13, 0, ENE_ETSK_MAJO_MAIN, 2, 1 },
    { 1, 1766, 3, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 0, 144, -30, -478, ENE_BOZE_WATER, 2, 1 },
    { 0, 213, -125, -408, ENE_BOZE, 2, 8388608 },
    { 0, 553, -10, -980, ENE_BOZE_UBRIDGE, 2, 0 },
    { 0, 553, -10, -930, ENE_BOZE_UBRIDGE, 2, 0 },
    { 0, 553, -10, -880, ENE_BOZE_UBRIDGE, 2, 0 },
    { 0, 553, -10, -830, ENE_BOZE_UBRIDGE, 2, 0 },
    { 0, 553, -10, -780, ENE_BOZE_UBRIDGE, 2, 0 },
    { 0, 553, -10, -730, ENE_BOZE_UBRIDGE, 2, 0 },
    { 0, 553, -10, -680, ENE_BOZE_UBRIDGE, 2, 0 },
    { 0, 553, -10, -630, ENE_BOZE_UBRIDGE, 2, 0 },
    { 0, 553, -10, -580, ENE_BOZE_UBRIDGE, 2, 0 },
    { 0, 553, -10, -530, ENE_BOZE_UBRIDGE, 2, 0 },
    { 0, 553, -10, -480, ENE_BOZE_UBRIDGE, 2, 0 },
    { 0, 553, -10, -430, ENE_BOZE_UBRIDGE, 2, 0 },
    { 0, 505, -10, -292, ENE_BOZE_UBRIDGE, 2, 896 },
    { 0, 470, -10, -257, ENE_BOZE_UBRIDGE, 2, 896 },
    { 0, 435, -10, -222, ENE_BOZE_UBRIDGE, 2, 896 },
    { 0, 400, -10, -187, ENE_BOZE_UBRIDGE, 2, 896 },
    { 0, 365, -10, -152, ENE_BOZE_UBRIDGE, 2, 896 },
    { 0, 330, -10, -117, ENE_BOZE_UBRIDGE, 2, 896 },
    { 0, 295, -10, -82, ENE_BOZE_UBRIDGE, 2, 896 },
    { 0, 555, 9, -1042, ENE_BOZE_UBRIDGE, 2, 897 },
    { 0, 553, 15, -1041, ENE_BOZE_UBRIDGE, 2, 126 },
};
ConnectData castle43PlacesEne0FAParam0[] = {
    { 30.0f, 88, 1, 1755.0f, 48.0f, 0.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData castle43PlacesEne0FAParam1[] = {
    { 30.0f, 90, 1, 55.0f, 252.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData castle43PlacesEne0FAParam2[] = {
    { 30.0f, 91, 1, 140.0f, 418.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef castle43Places[] = {
    { 1, 155, 15, 0, ENE_CONNECT, 2, (u32) &castle43PlacesEne0FAParam0 },
    { 3, 50, 315, 0, ENE_CONNECT, 2, (u32) &castle43PlacesEne0FAParam1 },
    { 2, 1375, 400, 0, ENE_CONNECT, 2, (u32) &castle43PlacesEne0FAParam2 },
    { 2, 360, 198, 0, ENE_TRANSFER, 2, 0 },
    { 3, 340, 328, 0, ENE_ITEM_OOIRI_MAIN, 2, 430 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SHIBAG_IWA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_WANYU_FIRE_MAIN, 0, 0 },
    { 1, 280, -9, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 2, 1 },
    { 1, 360, -9, 0, ENE_ETSK_YOROI_MAIN, 2, 1 },
    { 1, 520, -9, 0, ENE_ETSK_YOROI_MAIN, 2, 1 },
    { 1, 690, -39, -50, ENE_ETSK_MAJO_MAIN, 2, 5 },
    { 1, 720, -39, -50, ENE_ETSK_MAJO_MAIN, 2, 5 },
    { 1, 770, -39, -50, ENE_ETSK_MAJO_MAIN, 2, 6 },
    { 1, 720, -9, -5, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 760, -9, -5, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 770, -9, -5, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 810, -9, -5, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 820, -9, -5, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 5 },
    { 1, 1070, -37, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1095, -37, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1120, -37, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1150, -17, 0, ENE_ETSK_SHIBAG_MAIN, 2, 1 },
    { 1, 1180, 1, 0, ENE_ETSK_SHIBAG_MAIN, 2, 1 },
    { 1, 1465, 0, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1555, 19, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1530, 64, 0, ENE_ETSK_WANYU_MAIN, 2, 1 },
    { 1, 1290, 38, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 3, 1270, 38, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 3, 1235, 54, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 3, 1135, 103, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 7 },
    { 3, 800, 133, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 6 },
    { 3, 730, 150, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 3, 710, 160, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 3, 690, 170, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 3, 670, 170, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 3, 650, 180, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 3, 630, 190, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 3, 565, 190, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 6 },
    { 3, 495, 200, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 3, 475, 200, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 3, 455, 210, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 3, 435, 220, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 3, 415, 220, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 3, 395, 230, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 3, 275, 238, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 6 },
    { 2, 290, 170, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 2, 540, 268, 0, ENE_ETSK_WANYU_MAIN, 2, 1 },
    { 2, 730, 258, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 2, 885, 258, 0, ENE_ETSK_SHIBAG_MAIN, 2, 1 },
    { 2, 1065, 302, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 6 },
    { 2, 1210, 331, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 6 },
    { 0, -460, -7, -418, ENE_TRAP_BPROP, 2, 8388638 },
    { 0, -420, -7, -380, ENE_TRAP_BPROP, 2, 8388894 },
    { 0, -380, -7, -343, ENE_TRAP_BPROP, 2, 8389150 },
    { 0, -121, 95, -87, ENE_TRAP_BPROP, 2, 8388638 },
    { 0, -276, 129, -246, ENE_TRAP_BPROP, 2, 8388638 },
    { 0, -143, 200, -111, ENE_TRAP_BPROP, 2, 8388638 },
    { 0, 218, 276, 253, ENE_TRAP_BPROP, 2, 8388638 },
    { 0, 260, 294, 292, ENE_TRAP_BPROP, 2, 8388894 },
    { 0, 361, 323, 391, ENE_TRAP_BPROP, 2, 8389150 },
    { 0, 462, 353, 489, ENE_TRAP_BPROP, 2, 8389406 },
    { 0, -343, 255, -755, ENE_TRAP_BPROP, 2, 8388638 },
    { 0, -243, 253, -859, ENE_TRAP_BPROP, 2, 8388894 },
    { 0, -205, 273, -897, ENE_TRAP_BPROP, 2, 8389150 },
    { 0, -136, -30, -104, ENE_TRAP_BELCON, 2, -1048317 },
    { 0, -196, 94, -164, ENE_TRAP_BELCON, 2, 42008322 },
    { 0, -331, 124, -297, ENE_TRAP_BELCON, 2, 41943553 },
    { 0, -394, 124, -355, ENE_TRAP_BELCON, 2, 42007809 },
    { 0, -40, 223, -6, ENE_TRAP_BELCON, 2, 1113603 },
    { 0, 310, 293, 340, ENE_TRAP_BELCON, 2, 42007809 },
    { 0, 410, 322, 440, ENE_TRAP_BELCON, 2, 41944065 },
    { 0, -463, 181, -573, ENE_TRAP_BELCON, 2, 33555201 },
    { 0, -297, 229, -804, ENE_TRAP_BELCON, 2, 25230337 },
    { 0, -36, -15, -2, ENE_TRAP_BTOGE, 2, 8453888 },
};
ConnectData castle44PlacesEne0FAParam0[] = {
    { 30.0f, 89, 3, 90.0f, 288.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData castle44PlacesEne0FAParam1[] = {
    { 30.0f, 92, 1, 88.0f, -195.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef castle44Places[] = {
    { 1, 5, 275, 0, ENE_CONNECT, 2, (u32) &castle44PlacesEne0FAParam0 },
    { 1, 2380, 445, 0, ENE_CONNECT, 2, (u32) &castle44PlacesEne0FAParam1 },
    { 1, 1655, 370, 0, ENE_ETSK_GOLDNEKO_MAIN_FLAGS_0, 2, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_HIPEYE_UNKO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_HIPEYE_YUGE_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SHIBAG_IWA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_WANYU_FIRE_MAIN, 0, 0 },
    { 1, 340, 180, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 360, 190, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 380, 180, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 400, 170, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 420, 160, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 440, 150, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 720, 149, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 790, 140, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 820, 134, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 830, 174, 0, ENE_ETSK_WANYU_MAIN, 2, 1 },
    { 1, 1000, 130, 0, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 5 },
    { 1, 1107, 130, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1227, 130, 0, ENE_ETSK_SH_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1370, 135, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1400, 135, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1510, 160, 0, ENE_ETSK_WANYU_MAIN, 2, 1 },
    { 1, 1630, 130, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1760, 141, 0, ENE_ETSK_MAJO_MAIN, 2, 2 },
    { 1, 1780, 120, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1800, 120, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1875, 101, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 1930, 89, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 1955, 89, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 2, 1 },
    { 1, 2080, 155, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 2080, 274, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 2080, 382, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 2125, 400, 0, ENE_ETSK_SHIBAG_MAIN, 2, 1 },
    { 1, 2290, 410, 0, ENE_ETSK_HIPEYE_MAIN, 2, 1 },
    { 1, 2365, 410, 35, ENE_ETSK_YOROI_MAIN, 2, 3 },
    { 1, 2365, 410, -35, ENE_ETSK_YOROI_MAIN, 2, 3 },
    { 2, 153, 460, 0, ENE_ETSK_RHANI_MAIN_HOURS_ANY, 2, 1 },
    { 0, -15, 144, -4, ENE_TRAP_BPROP, 2, 30 },
    { 0, 50, 154, -2, ENE_TRAP_BPROP, 2, 286 },
    { 0, 456, 124, -44, ENE_TRAP_BPROP, 2, 3145758 },
    { 0, 573, 126, -89, ENE_TRAP_BPROP, 2, 4981022 },
    { 0, 682, 133, -142, ENE_TRAP_BPROP, 2, 5440030 },
    { 0, 951, 127, -231, ENE_TRAP_BPROP, 2, 798 },
    { 0, 1367, 120, -270, ENE_TRAP_BPROP, 2, 30 },
    { 0, 1367, 210, -270, ENE_TRAP_BPROP, 2, 45 },
    { 0, 1367, 320, -270, ENE_TRAP_BPROP, 2, 45 },
    { 0, 1308, 370, -270, ENE_TRAP_BPROP, 2, 798 },
    { 0, 1249, 360, -270, ENE_TRAP_BPROP, 2, 542 },
    { 0, 1190, 350, -270, ENE_TRAP_BPROP, 2, 286 },
    { 0, 1563, 390, -270, ENE_TRAP_BPROP, 2, 286 },
    { 0, 323, 78, -31, ENE_TRAP_GUPPA, 2, 0 },
    { 0, 521, 78, -73, ENE_TRAP_GUPPA, 2, 2752512 },
    { 0, 627, 78, -123, ENE_TRAP_GUPPA, 2, 5570560 },
    { 0, 820, 96, -224, ENE_TRAP_GUPPA, 2, 2752512 },
    { 0, 888, 81, -240, ENE_TRAP_GUPPA, 2, 0 },
    { 0, 1013, 81, -240, ENE_TRAP_GUPPA, 2, 0 },
};
ConnectData castle45PlacesEne0FAParam0[] = {
    { 30.0f, 89, 2, 1330.0f, 374.0f, 0.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData castle45PlacesEne0FAParam1[] = {
    { 30.0f, 94, 1, 95.0f, -65.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef castle45Places[] = {
    { 1, 90, 445, 0, ENE_CONNECT, 2, (u32) &castle45PlacesEne0FAParam0 },
    { 2, 25, 30, 0, ENE_CONNECT, 2, (u32) &castle45PlacesEne0FAParam1 },
    { 3, 835, -170, 0, ENE_CONNECT, 2, (u32) &castle44PlacesEne0FAParam1 },
    { 1, 1329, 111, 0, ENE_TRANSFER, 2, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_WANYU_FIRE_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_BOATSKEL_HEAD_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_WAGA_KASA, 0, 0 },
    { 0, 0, 0, 0, ENE_WAGA_NORU, 0, 0 },
    { 0, 0, 0, 0, ENE_NORU_MARK, 0, 0 },
    { 1, 200, 400, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 250, 427, 0, ENE_ETSK_WANYU_MAIN, 2, 0 },
    { 1, 260, 390, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 330, 406, 0, ENE_ETSK_WANYU_MAIN, 2, 0 },
    { 1, 360, 370, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 440, 350, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 1, 475, 334, -55, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 490, 320, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 1, 550, 310, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 1, 600, 298, -55, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 610, 290, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 1, 630, 290, 0, ENE_ETSK_YADO_MAIN, 2, 2 },
    { 1, 725, 264, -55, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 750, 247, 0, ENE_ETSK_YADO_MAIN, 2, 1 },
    { 1, 820, 226, 0, ENE_ETSK_YADO_MAIN, 2, 1 },
    { 1, 850, 227, -55, ENE_ETSK_BOATSKEL_MAIN, 2, 1 },
    { 1, 985, 190, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1025, 180, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1080, 200, 0, ENE_ETSK_WANYU_MAIN, 2, 0 },
    { 1, 1140, 130, 0, ENE_WAGA, 2, 1 },
    { 1, 1200, 120, 0, ENE_WAGA, 2, 1 },
    { 1, 1240, 120, 0, ENE_WAGA, 2, 1 },
    { 2, 660, 60, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 2, 630, 60, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 2, 600, 60, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 2, 570, 60, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 2, 530, 30, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 2, 500, 30, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 2, 470, 30, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 2, 440, 30, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 2, 400, 10, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 4 },
    { 2, 370, 10, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 4 },
    { 2, 340, 10, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 4 },
    { 2, 310, 10, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 4 },
    { 2, 270, 0, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 4 },
    { 2, 240, 0, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 4 },
    { 2, 210, 0, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 4 },
    { 2, 180, 0, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 4 },
    { 2, 65, -60, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 2, 40, -60, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 2, 1 },
    { 3, 100, -87, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 3, 150, -87, 0, ENE_WAGA, 2, 0 },
    { 3, 160, -87, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 3, 260, -90, -5, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 3, 290, -90, -5, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 3, 320, -90, -5, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 3, 360, -100, -5, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 3, 390, -100, -5, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 3, 420, -100, -5, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 3, 440, -93, 0, ENE_ETSK_WANYU_MAIN, 2, 1 },
    { 3, 540, -109, 0, ENE_ETSK_WANYU_MAIN, 2, 1 },
    { 3, 640, -126, 0, ENE_ETSK_WANYU_MAIN, 2, 1 },
    { 2, 850, 61, 0, ENE_IMP_OBJ_BALL, 2, 0 },
    { 0, -239, 41, 529, ENE_SASROCK, 2, 0 },
    { 0, -239, 81, 529, ENE_SASROCK, 2, 0 },
};
ConnectData castle46Places2AEne0FAParam[] = {
    { 30.0f, 90, 1, 2335.0f, 415.0f, 0.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData castle46PlacesEne0FAParam[] = {
    { 30.0f, 93, 1, 210.0f, -10.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData castle46Places2BEne0FAParam[] = {
    { 30.0f, 91, 3, 795.0f, -200.0f, 0.0f, 3.1415927f, 0x0000, 0, 0 },
};
EneDef castle46Places[] = {
    { 1, 3900, 260, 0, ENE_CONNECT, 2, (u32) &castle46PlacesEne0FAParam },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_HANI_FIRE_HOURS_ANY, 0, 0 },
    { 0, 0, 0, 0, ENE_WANYU_FIRE_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_BIWA_FIRE, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_BIWA_KUMO, 0, 0 },
    { 0, 0, 0, 0, ENE_SHIBAG_IWA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_WAGA_KASA, 0, 0 },
    { 0, 0, 0, 0, ENE_WAGA_NORU, 0, 0 },
    { 0, 0, 0, 0, ENE_NORU_MARK, 0, 0 },
    { 1, 225, -240, -50, ENE_ETSK_MAJO_MAIN, 2, 5 },
    { 1, 245, -240, -50, ENE_ETSK_MAJO_MAIN, 2, 6 },
    { 1, 250, -200, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 345, -191, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 3 },
    { 1, 425, -199, 0, ENE_WAGA, 2, 0 },
    { 1, 480, -191, 0, ENE_ETSK_SHIBAG_MAIN, 2, 1 },
    { 1, 660, -176, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 690, -176, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 740, -150, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 800, -140, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 900, -140, 0, ENE_ETSK_RHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 950, -140, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1125, -104, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1200, -104, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1220, -104, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1240, -104, 0, ENE_ETSK_SHIBAG_MAIN, 2, 1 },
    { 1, 1335, -80, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 5 },
    { 1, 1450, -69, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1470, -69, 0, ENE_ETSK_YOROI_MAIN, 2, 1 },
    { 1, 1820, -19, 0, ENE_ETSK_YOROI_MAIN, 2, 1 },
    { 1, 1980, 40, 0, ENE_ETSK_BIWA_MAIN, 2, 1 },
    { 1, 1990, 15, 0, ENE_WAGA, 2, 1 },
    { 1, 2060, 15, 0, ENE_WAGA, 2, 1 },
    { 1, 2070, 50, 0, ENE_ETSK_BIWA_MAIN, 2, 1 },
    { 1, 2390, 60, 0, ENE_ETSK_S_STONE_MAIN, 2, 0 },
    { 1, 2460, 80, 0, ENE_ETSK_S_STONE_MAIN, 2, 0 },
    { 1, 2540, 96, 0, ENE_ETSK_S_STONE_MAIN, 2, 0 },
    { 1, 2600, 96, 0, ENE_ETSK_S_STONE_MAIN, 2, 0 },
    { 1, 2630, 135, 0, ENE_ETSK_WANYU_MAIN, 2, 0 },
    { 1, 2670, 95, 0, ENE_ETSK_S_STONE_MAIN, 2, 0 },
    { 1, 2755, 200, 0, ENE_ETSK_WANYU_MAIN, 2, 0 },
    { 1, 2855, 184, 0, ENE_ETSK_WANYU_MAIN, 2, 0 },
    { 1, 2950, 145, 0, ENE_WAGA, 2, 0 },
    { 1, 3170, 180, -5, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 3195, 180, -5, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 3220, 180, -5, ENE_ETSK_KT_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 3230, 215, -5, ENE_ETSK_BIWA_MAIN, 2, 1 },
    { 1, 3295, 194, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3360, 220, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3380, 220, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3430, 231, 0, ENE_ETSK_BZ_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3450, 261, -5, ENE_ETSK_BIWA_MAIN, 2, 1 },
    { 1, 3525, 242, 0, ENE_POT, 2, 0 },
    { 1, 3555, 242, 0, ENE_POT, 2, 0 },
    { 1, 3630, 257, 0, ENE_POT, 2, 0 },
    { 1, 3650, 257, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 2, 1 },
    { 1, 3670, 257, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 3760, -246, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 3785, -246, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 3810, 281, 0, ENE_ETSK_BIWA_MAIN, 2, 1 },
    { 0, 314, -116, 174, ENE_TRAP_BPROP, 2, 60817438 },
    { 0, 284, -105, 219, ENE_TRAP_BPROP, 2, 59900702 },
    { 0, -176, -45, 315, ENE_TRAP_BPROP, 2, 61472798 },
    { 0, -222, -16, 282, ENE_TRAP_BPROP, 2, 60228382 },
    { 0, -316, 42, -177, ENE_TRAP_BPROP, 2, 61538334 },
    { 0, -285, 65, -226, ENE_TRAP_BPROP, 2, 60293918 },
    { 0, 182, 124, -315, ENE_TRAP_BPROP, 2, 61603870 },
    { 0, 226, 137, -287, ENE_TRAP_BPROP, 2, 60556062 },
    { 1, 618, -167, -18, ENE_TRAP_FIREBELL, 2, 0 },
    { 1, 932, -130, -18, ENE_TRAP_FIREBELL, 2, 0 },
    { 1, 1302, -84, -18, ENE_TRAP_FIREBELL, 2, 0 },
    { 1, 1363, -69, -18, ENE_TRAP_FIREBELL, 2, 0 },
    { 1, 2100, 28, -18, ENE_TRAP_FIREBELL, 2, 0 },
    { 1, 2100, 48, -18, ENE_TRAP_FIREBELL, 2, 0 },
    { 1, 2305, 82, -18, ENE_TRAP_FIREBELL, 2, 0 },
    { 1, 2340, 82, -18, ENE_TRAP_FIREBELL, 2, 0 },
    { 1, 2375, 82, -18, ENE_TRAP_FIREBELL, 2, 0 },
};
EneDef castle46Places2A[] = {
    { 1, 35, -170, 0, ENE_CONNECT, 2, (u32) &castle46Places2AEne0FAParam },
};
EneDef castle46Places2B[] = {
    { 1, 35, -170, 0, ENE_CONNECT, 2, (u32) &castle46Places2BEne0FAParam },
};
ConnectData castle47PlacesEne0FAParam0[] = {
    { 30.0f, 92, 1, 3850.0f, 235.0f, 0.0f, 3.1415927f, 0x0000, 0, 0 },
};
ConnectData castle47PlacesEne0FAParam1[] = {
    { 40.0f, 99, 1, 0.0f, 48.0f, 0.0f, 0.0f, 0x000A, 0, 0 },
};
EneDef castle47Places[] = {
    { 1, 160, 12, 0, ENE_CONNECT, 2, (u32) &castle47PlacesEne0FAParam0 },
    { 1, 1450, 0, 0, ENE_CONNECT, 2, (u32) &castle47PlacesEne0FAParam1 },
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_HIPEYE_UNKO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_HIPEYE_YUGE_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_IMPROCK_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SHIBAG_IWA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_WAGA_KASA, 0, 0 },
    { 0, 0, 0, 0, ENE_WAGA_NORU, 0, 0 },
    { 0, 0, 0, 0, ENE_NORU_MARK, 0, 0 },
    { 1, 260, 0, -50, ENE_ETSK_YOROI_MAIN, 2, 4 },
    { 1, 290, 0, -50, ENE_ETSK_YOROI_MAIN, 2, 4 },
    { 1, 320, 0, -50, ENE_ETSK_YOROI_MAIN, 2, 4 },
    { 1, 350, 0, -50, ENE_ETSK_YOROI_MAIN, 2, 4 },
    { 1, 400, 0, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 440, 0, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 470, 0, 0, ENE_WAGA, 2, 0 },
    { 1, 620, 0, 0, ENE_ETSK_MAJO_MAIN, 2, 1 },
    { 1, 670, 0, 0, ENE_ETSK_MAJO_MAIN, 2, 2 },
    { 1, 670, 0, -22, ENE_ETSK_YOROI_MAIN, 2, 2 },
    { 1, 700, 0, -22, ENE_ETSK_YOROI_MAIN, 2, 2 },
    { 1, 730, 0, -22, ENE_ETSK_YOROI_MAIN, 2, 2 },
    { 1, 760, 0, -22, ENE_ETSK_YOROI_MAIN, 2, 2 },
    { 1, 810, 0, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 830, 0, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 845, 67, -162, ENE_IMPROCK_J, 2, 2 },
    { 1, 930, 67, -162, ENE_IMPROCK_J, 2, 2 },
    { 1, 950, -13, 0, ENE_ETSK_SHIBAG_MAIN, 2, 1 },
    { 1, 1015, 67, -162, ENE_IMPROCK_J, 2, 2 },
    { 1, 1035, 0, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 1110, 0, 0, ENE_ETSK_YOROI_MAIN, 2, 1 },
    { 1, 1210, 0, -50, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1240, 0, -50, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1270, 0, -50, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1296, 0, 0, ENE_ETSK_DANGO_MAIN, 2, 1 },
    { 1, 1319, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 2, 1 },
    { 1, 1380, 10, 0, ENE_ETSK_HIPEYE_MAIN, 2, 1 },
};
ConnectData castle48PlacesEne0FAParam[] = {
    { 30.0f, 91, 2, 75.0f, 4.0f, 0.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef castle48Places[] = {
    { 1, 45, -40, 0, ENE_CONNECT, 2, (u32) &castle48PlacesEne0FAParam },
    { 1, 594, -144, 0, ENE_ETSK_GOLDNEKO_MAIN_FLAGS_0, 2, 0 },
    { 1, 300, -103, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 320, -103, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 340, -103, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 360, -106, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 380, -110, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 400, -114, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 420, -118, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 440, -118, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 460, -118, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 490, -127, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 510, -127, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 530, -129, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 550, -133, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
    { 1, 570, -137, 0, ENE_ETSK_KOBAN_MAIN, 2, 1 },
};

void castle41Init(void) {
    stageBuildBasic(0x3B, 1, 500.0f, -20.0f, 0.0f, (u32) castle41Places, 0x2A);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x50);
    if (gGameWork.tunePart <= 0) {
        soundPlayAfterOne(0xAB);
        gGameWork.tunePart = 1;
    }
    gGameWork.time = 12 << 16;
    camSunSetClockStop(1);
    camSunSetProgram(9);
}

void castle42Init(void) {
    stageBuildBasic(0x3D, 1, 80.0f, 50.0f, 0.0f, (u32) castle42Places, 0x2E);
    camAiSetFar(1100);
    camAiAddLimit(1, -1, 0x4B);
    camAiAddLimit(1, 1, 0x6D6);
    if (gGameWork.tunePart <= 0) {
        soundPlayAfterOne(0xAB);
        gGameWork.tunePart = 1;
    }
    gGameWork.time = 12 << 16;
    camSunSetClockStop(1);
    camSunSetProgram(9);
}

void castle43Init(void) {
    stageBuildBasic(0x41, 1, 195.0f, -5.0f, 0.0f, (u32) castle43Places, 0x52);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 200);
    camAiAddLimit(3, -1, 0x69);
    if (gGameWork.tunePart <= 0) {
        soundPlayAfterOne(0xAB);
        gGameWork.tunePart = 1;
    }
    gGameWork.time = 12 << 16;
    camSunSetClockStop(1);
    camSunSetProgram(9);
}

void castle44Init(void) {
    stageBuildBasic(0x46, 1, 55.0f, 252.0f, 0.0f, (u32) castle44Places, 0x3E);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x3C);
    camAiAddLimit(1, 1, 0x929);
    if (gGameWork.tunePart < 2) {
        soundPlayAfterOne(0xAC);
        gGameWork.tunePart = 2;
    }
    gTimeOfDay = 12 << 16;
    camSunSetClockStop(1);
    camSunSetProgram(9);
    gGameWork.entryFrom = 0;
}

void castle45Init(void) {
    stageBuildBasic2Atk150(0x4A, 1, 140.0f, 418.0f, 0.0f, (u32) castle45Places, 0x45, 0, 0);
    camAiSetFar(1000);
    camAiAddLimit(1, -1, 0x82);
    camAiAddLimit(2, -1, 0x4B);
    camAiAddLimit(2, 1, 0x34B);
    if (gGameWork.tunePart < 2) {
        soundPlayAfterOne(0xAC);
        gGameWork.tunePart = 2;
    }
    gTimeOfDay = 12 << 16;
    camSunSetClockStop(1);
    camSunSetProgram(9);
    gGameWork.entryFrom = 1;
}

void castle46Init(void) {
    stageBuildBasic2(0x4B, 1, 88.0f, -195.0f, 0.0f, (u32) castle46Places, 0x51,
                     (gGameWork.entryFrom == 0) ? (u32) castle46Places2A : (u32) castle46Places2B, 1);
    camAiSetFar(1000);
    camAiAddLimit(1, -1, 0x55);
    if (gGameWork.tunePart < 2) {
        soundPlayAfterOne(0xAC);
        gGameWork.tunePart = 2;
    }
    gTimeOfDay = 12 << 16;
    camSunSetClockStop(1);
    camSunSetProgram(9);
}

void castle47Init(void) {
    stageBuildBasic(0x4F, 1, 210.0f, -10.0f, 0.0f, (u32) castle47Places, 0x28);
    camAiSetFar(800);
    camAiAddLimit(1, 1, 0x57D);
    if (gGameWork.tunePart < 3) {
        soundPlayAfterOne(0xAD);
        gGameWork.tunePart = 3;
    }
    gGameWork.time = 12 << 16;
    camSunSetClockStop(1);
    camSunSetProgram(9);
}

void castle48Init(void) {
    stageBuildBasic(0x53, 1, 95.0f, -65.0f, 0.0f, (u32) castle48Places, 0x10);
    camAiSetFar(800);
    camAiAddLimit(1, -1, 0x55);
    if (gGameWork.tunePart < 2) {
        soundPlayAfterOne(0xAC);
        gGameWork.tunePart = 2;
    }
    gGameWork.time = 12 << 16;
    camSunSetClockStop(1);
    camSunSetProgram(9);
}
