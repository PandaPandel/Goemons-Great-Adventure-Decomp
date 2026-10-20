#include "tlb.h"
#include "tlb/tlb_0322.h"

/* The staff roll's stages (file 322): stages 0x100-0x109, "STAFF 0" to "STAFF 9" in the stage
 * list (file 13, assets/tables/table_0013.c), in that order here. Each builder sets the players
 * up, plays the stage's recorded input (C-right held on pad 1 records a new one instead) and
 * builds the stage with the gauge off; file 180's "staff0" task goes from each page to the next
 * page's stage (its staffPageStages: 0x101, 0x102, 0x103, 0x105, 0x104, ...). "STAFF 9" builds no
 * stage: it starts page 9 itself.
 *
 * The game's file was linked from eight objects, tlb_0322_0 to tlb_0322_7 (one or two builders
 * each). Six cuts show in the code: the nops after a builder whose size is not a multiple of 16
 * are its object's padding (nisitenma_archive.py pad-cuts and recut, measured earlier). Two
 * more, before stageStaff3Init and stageStaff8Init, show only in the data, as the builders before
 * them end on a 16-byte boundary: each object's .data is padded to 16 with zero words, so the 1-3
 * zero words after a stage's last placements end its object, and the next object's data starts
 * with its stage's demo input. In the six-object cut, the demo input headers of stages 1, 2, 5, 6
 * and 7 pointed into the object before theirs, past that object's last placements. */

extern DemoKey stageStaff0DemoInputPlayer1[];
extern DemoKey stageStaff0DemoInputPlayer2[];
extern DemoKey stageStaff0DemoInputPlayer3[];
extern DemoKey stageStaff0DemoInputPlayer4[];
extern Tlb99Data stageStaff0Placements2Ene0F3Param[];
extern u32 stageStaff0PlacementsEne05CParam[];
DemoKey stageStaff0DemoInputPlayer1[] = {
    { 0x0000001B, { 0x0001, 75, 7 } },
    { 0x0000000E, { 0x0000, 75, 7 } },
    { 0x00000008, { 0x8000, 75, 7 } },
    { 0x00000010, { 0x0000, 75, 7 } },
    { 0x00000004, { 0x8000, 75, 7 } },
    { 0x00000001, { 0x0000, 75, 7 } },
    { 0x00000004, { 0x8000, 75, 7 } },
    { 0x00000020, { 0x0000, 75, 7 } },
    { 0x00000004, { 0x4000, 75, 7 } },
    { 0x0000001A, { 0x0000, 75, 7 } },
    { 0x00000002, { 0x8000, 75, 7 } },
    { 0x00000001, { 0x8000, 75, 8 } },
    { 0x00000002, { 0x8000, 75, 9 } },
    { 0x00000019, { 0x0000, 75, 9 } },
    { 0x00000004, { 0x4000, 75, 9 } },
    { 0x00000035, { 0x0000, 75, 9 } },
    { 0x00000004, { 0x0000, 75, 10 } },
    { 0x00000003, { 0x8000, 75, 10 } },
    { 0x00000003, { 0x8000, 75, 11 } },
    { 0x00000014, { 0x0000, 75, 11 } },
    { 0x00000005, { 0x8000, 75, 11 } },
    { 0x00000027, { 0x0000, 75, 12 } },
    { 0x00000003, { 0x8000, 75, 12 } },
    { 0x00000001, { 0x8000, 75, 13 } },
    { 0x00000002, { 0x0000, 75, 13 } },
    { 0x00000006, { 0x8000, 75, 13 } },
    { 0x00000001, { 0x8000, 75, 14 } },
    { 0x00000001, { 0x0000, 75, 14 } },
    { 0x00000004, { 0x4000, 75, 14 } },
    { 0x00000003, { 0x4000, 75, 15 } },
    { 0x00000002, { 0x4000, 74, 17 } },
    { 0x0000000D, { 0x0000, 74, 17 } },
    { 0x00000002, { 0x8000, 74, 17 } },
    { 0x00000001, { 0x8000, 74, 18 } },
    { 0x00000004, { 0x0000, 74, 18 } },
    { 0x00000002, { 0x8000, 74, 18 } },
    { 0x0000002D, { 0x0000, 74, 18 } },
    { 0x00000006, { 0x8000, 74, 18 } },
    { 0x00000002, { 0x0000, 74, 18 } },
    { 0x00000002, { 0x0002, 74, 18 } },
    { 0x00000007, { 0x0002, 73, 18 } },
    { 0x00000009, { 0x0000, 73, 18 } },
    { 0x000001F4 },
    { 0x00000000 },
};
DemoKey stageStaff0DemoInputPlayer2[] = {
    { 0x000001C2 },
    { 0x000001F4 },
    { 0x00000000 },
};
DemoKey stageStaff0DemoInputPlayer3[] = {
    { 0x000001C2 },
    { 0x000001F4 },
    { 0x00000000 },
};
DemoKey stageStaff0DemoInputPlayer4[] = {
    { 0x000001C2 },
    { 0x000001F4 },
    { 0x00000000 },
};
u32 stageStaff0DemoInput[] = { 0x00000004, (u32) &stageStaff0DemoInputPlayer1, (u32) &stageStaff0DemoInputPlayer2, (u32) &stageStaff0DemoInputPlayer3, (u32) &stageStaff0DemoInputPlayer4 };
Tlb99Data stageStaff0Placements2Ene0F3Param[] = {
    { 0, 1, { 1, -1, -1, -1 }, 152 },
};
u32 stageStaff0PlacementsEne05CParam[] = { 0x00000004, 0x00000287, 0x000000C8, 0x000002EE, 0x00000000 };
EneDef stageStaff0Placements[] = {
    { 0, 0, 0, 0, ENE_BASHI, 0, 0 },
    { 0, 0, 0, 0, ENE_MIMI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_KASA_MAIN_HOURS_ANY, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_IMPROCK_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SIROBZ, 0, 0 },
    { 0, 0, 0, 0, ENE_UMA_NORU, 0, 0 },
    { 0, 0, 0, 0, ENE_UMA_HANI_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_UMA_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_NORU_MARK, 0, 0 },
    { 0, 0, 0, 0, ENE_NORU_MARK, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_LANTERN_MAIN, 2, 1 },
    { 0, 0, 0, 0, ENE_ETSK_BETO_MAIN, 2, 1 },
    { 0, 0, 0, 0, ENE_GYUKI, 2, 0 },
    { 0, 0, 0, 0, ENE_BIG_BOMB, 0, 0 },
    { 1, 270, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 320, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 340, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 420, 47, 0, ENE_SIRO_J_HOURS_ANY, 2, 2 },
    { 1, 500, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 550, 52, 0, ENE_SIRO_J_HOURS_ANY, 2, 2 },
    { 1, 700, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 730, 55, 0, ENE_SIRO_J_HOURS_ANY, 2, 2 },
    { 1, 770, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 880, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 900, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 910, 57, 0, ENE_SIRO_J_HOURS_5_19, 2, 2 },
    { 1, 980, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 300, 30, 0, ENE_ETSK_BETO_MAIN, 2, 3 },
    { 1, 400, 30, 0, ENE_ETSK_BETO_MAIN, 2, 3 },
    { 1, 660, 30, 0, ENE_ETSK_BETO_MAIN, 2, 1 },
    { 1, 845, 30, 0, ENE_ETSK_BETO_MAIN, 2, 4 },
    { 1, 930, 50, 0, ENE_ETSK_LANTERN_MAIN, 2, 1 },
    { 2, 440, 176, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 2, 470, 176, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 2, 525, 176, 0, ENE_ETSK_KOHANI_MAIN_HOURS_ANY, 2, 2 },
    { 2, 650, 219, 0, ENE_SIRO_J_HOURS_5_19, 2, 1 },
    { 2, 620, 220, 0, ENE_ETSK_LANTERN_MAIN, 2, 1 },
    { 1, 1135, 0, 0, ENE_KAPPA_JENEL_HOURS_5_19, 2, 1 },
    { 1, 1164, 16, 0, ENE_MIMI, 2, 3 },
    { 1, 1215, 0, 0, ENE_KAPPA_JENEL_HOURS_5_19, 2, 1 },
    { 1, 1236, 16, 0, ENE_MIMI, 2, 3 },
    { 1, 1390, 40, 0, ENE_SIRO_J_HOURS_ANY, 2, 2 },
    { 1, 1465, 20, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 1523, 16, 0, ENE_MIMI, 2, 3 },
    { 1, 1320, 50, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 1, 1430, 10, 0, ENE_ETSK_BETO_MAIN, 2, 4 },
    { 1, 1480, 10, 0, ENE_ETSK_BETO_MAIN, 2, 4 },
    { 1, 1740, 37, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 1825, 56, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 1920, 72, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 5 },
    { 1, 1930, 102, 0, ENE_SIRO_J_HOURS_ANY, 2, 2 },
    { 1, 2070, 72, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 2080, 102, 0, ENE_SIRO_J_HOURS_ANY, 2, 0 },
    { 1, 2120, 72, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 1740, 37, 0, ENE_ETSK_BETO_MAIN, 2, 1 },
    { 1, 1825, 56, 0, ENE_ETSK_BETO_MAIN, 2, 1 },
    { 1, 2200, 85, 0, ENE_ETSK_KAPPA_MAIN_HOURS_5_19, 2, 5 },
    { 1, 2300, 100, 0, ENE_ETSK_KAPPA_MAIN_HOURS_5_19, 2, 4 },
    { 1, 2400, 100, 0, ENE_ETSK_KAPPA_MAIN_HOURS_5_19, 2, 5 },
    { 1, 2485, 119, 0, ENE_ETSK_KAPPA_MAIN_HOURS_5_19, 2, 4 },
    { 1, 2515, 122, 0, ENE_ETSK_KAPPA_MAIN_HOURS_5_19, 2, 4 },
    { 1, 2350, 130, 0, ENE_GYUKI, 2, 0 },
    { 0, 0, 0, 0, ENE_BIG_BOMB, 0, 0 },
    { 1, 2700, 150, 0, ENE_UMA_SET_2F_HOURS_ANY, 2, 0 },
    { 1, 2860, 150, 0, ENE_UMA_SET_2F_HOURS_ANY, 2, 0 },
    { 1, 2920, 119, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 3016, 119, -20, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 3032, 119, -20, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 3048, 119, -20, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 3064, 119, -20, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 3040, 119, 0, ENE_IMPROCK_J, 2, 1 },
    { 1, 3040, 119, 70, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 3210, 119, 0, ENE_ETSK_KOMIMI_MAIN, 2, 1 },
    { 1, 3240, 119, 0, ENE_ETSK_KOMIMI_MAIN, 2, 1 },
    { 1, 3270, 119, 0, ENE_ETSK_KOMIMI_MAIN, 2, 1 },
    { 1, 3300, 119, 0, ENE_ETSK_KOMIMI_MAIN, 2, 1 },
    { 1, 3380, 99, 0, ENE_ETSK_KOMIMI_MAIN, 2, 1 },
    { 1, 3470, 99, 0, ENE_ETSK_KOMIMI_MAIN, 2, 1 },
    { 1, 3450, 99, 0, ENE_ETSK_KOMIMI_MAIN, 2, 1 },
    { 1, 3485, 63, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 3 },
    { 1, 3590, 102, 0, ENE_SIRO_J_HOURS_ANY, 2, 2 },
    { 3, 160, 185, 0, ENE_ETSK_RHANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 3760, 132, 0, ENE_SIRO_J_HOURS_ANY, 2, 0 },
    { 1, 3820, 132, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 3800, 102, 0, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 6 },
    { 1, 3840, 102, 8, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 6 },
    { 1, 3840, 102, -8, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 6 },
    { 1, 3870, 102, 8, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 6 },
    { 1, 3870, 102, -8, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 6 },
    { 1, 3900, 102, 8, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 6 },
    { 1, 3900, 102, -8, ENE_ETSK_TB_HANI_MAIN_HOURS_ANY, 2, 6 },
    { 1, 3955, 116, 0, ENE_ETSK_KONURI_MAIN_HOURS_ANY, 2, 0 },
    { 1, 4040, 101, 0, ENE_ETSK_KAPPA_MAIN_HOURS_5_19, 2, 5 },
    { 1, 4090, 101, 0, ENE_ETSK_KAPPA_MAIN_HOURS_5_19, 2, 4 },
    { 1, 4175, 101, 0, ENE_ETSK_NURI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 3990, 120, 0, ENE_ETSK_BETO_MAIN, 2, 3 },
    { 1, 4012, 110, 0, ENE_ETSK_BETO_MAIN, 2, 5 },
    { 1, 4040, 122, 0, ENE_ETSK_LANTERN_MAIN, 2, 1 },
    { 1, 4350, 101, 0, ENE_ETSK_KONURI_MAIN_HOURS_ANY, 2, 0 },
    { 1, 4370, 101, 0, ENE_ETSK_KONURI_MAIN_HOURS_ANY, 2, 0 },
    { 1, 4390, 101, 0, ENE_ETSK_KONURI_MAIN_HOURS_ANY, 2, 0 },
    { 1, 4410, 101, 0, ENE_ETSK_KONURI_MAIN_HOURS_ANY, 2, 0 },
    { 1, 4430, 101, 0, ENE_ETSK_KONURI_MAIN_HOURS_ANY, 2, 0 },
    { 1, 4510, 82, -40, ENE_ETSK_NURI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 4560, 82, 40, ENE_ETSK_NURI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 4610, 82, -40, ENE_ETSK_NURI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 4695, 82, 0, ENE_ETSK_NURI_MAIN_HOURS_ANY, 2, 2 },
    { 1, 4770, 64, 0, ENE_ETSK_HANI_MAIN_HOURS_5_19, 2, 1 },
    { 1, 3990, 64, 0, ENE_ETSK_BETO_MAIN, 2, 1 },
    { 1, 4920, 27, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 6 },
    { 1, 4970, 27, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 6 },
    { 1, 5000, 27, 0, ENE_ETSK_KAPPA_MAIN_HOURS_ANY, 2, 6 },
    { 1, 5080, 70, 0, ENE_ETSK_LANTERN_MAIN, 2, 2 },
    { 1, 83, 17, 0, ENE_TANU_WALL_FLAGS_0, 0, 0 },
    { 1, 560, 65, 0, ENE_BPOLE, 2, 0 },
    { 1, 620, 90, 0, ENE_BPOLE, 2, 0 },
    { 1, 680, 115, 0, ENE_BPOLE, 2, 0 },
    { 1, 740, 140, 0, ENE_BPOLE, 2, 0 },
    { 1, 800, 165, 0, ENE_BPOLE, 2, 0 },
    { 0, 628, 4, 1623, ENE_WATER_POOL_FLAGS_2, 2, 1 },
    { 0, 583, 39, 498, ENE_WATER_POOL_FLAGS_2, 2, 2818049 },
    { 0, -269, 56, -782, ENE_WATER_POOL_FLAGS_2, 2, 3997697 },
    { 0, -519, 32, -1659, ENE_WATER_POOL_FLAGS_2, 2, 2293761 },
    { 0, 69, 86, -274, ENE_WATER_POOL_FLAGS_1, 2, 41025538 },
    { 0, -21, 64, -401, ENE_WATER_POOL_FLAGS_1, 2, 40108034 },
    { 0, 412, 197, 1735, ENE_WATER_SPLASH, 2, 16777219 },
    { 0, -16, 248, -155, ENE_WATER_SPLASH, 2, 29949956 },
    { 0, 69, 86, -274, ENE_WATER_SPLASH, 2, 41025541 },
    { 0, 0, 100, 0, ENE_WS_MASTER2, 3, (u32) &stageStaff0PlacementsEne05CParam },
    { 0, 481, 0, 1737, ENE_WS_SOUND2, 3, (u32) &stageStaff0PlacementsEne05CParam },
    { 0, -42, 82, -196, ENE_WS_SOUND2, 3, (u32) &stageStaff0PlacementsEne05CParam },
    { 0, 0, 0, 0, ENE_HOTARU, 2, 0 },
    { 0, 0, 1, 0, ENE_HOTARU_MASTER, 2, 0 },
    { 1, 3750, 102, 0, ENE_IMP_OBJ_BALL, 2, 0 },
    { 0, 0, 0, 0, ENE_STAFF_CINESCO, 2, 0 },
    { 0, 0, 10, 0, ENE_STAFF_PAGE, 2, 0 },
};
EneDef stageStaff0Placements2[] = {
    { 0, 0, 0, 0, ENE_POT_FRAGMENT, 0, 0 },
    { 2, 675, 200, 0, ENE_POT, 2, 0 },
    { 3, 140, 188, 0, ENE_POT, 2, 0 },
    { 1, 5230, 60, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 5260, 72, 0, ENE_SIRO_J_HOURS_ANY, 2, 0 },
    { 1, 5330, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 5360, 45, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 5400, 80, 0, ENE_SIRO_J_HOURS_ANY, 2, 2 },
    { 1, 5430, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 5480, 55, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 5500, 72, 0, ENE_SIRO_J_HOURS_ANY, 2, 0 },
    { 1, 5520, 55, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 5550, 80, 0, ENE_SIRO_J_HOURS_ANY, 2, 2 },
    { 1, 5670, 50, 0, ENE_ETSK_HANI_MAIN_HOURS_ANY, 2, 1 },
    { 1, 5600, 90, 0, ENE_SIRO_J_HOURS_ANY, 2, 2 },
    { 0, 0, 0, 0, ENE_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_KOBAN, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR_MOJI, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR, 0, 0 },
    { 1, 5840, 90, 0, ENE_TANU_J, 2, (u32) &stageStaff0Placements2Ene0F3Param },
    { 0, 0, 0, 0, ENE_EPRG_TANU, 0, 0 },
};

void stageStaff0Init(void) {
    s32 pad;

    gTimeOfDay = 10 << 16;
    gSaveData.players[0].lives = 1;
    gSaveData.players[0].chara = 0;
    gSaveData.players[0].life = 3;
    gSaveData.players[0].armor = 3;
    gSaveData.players[0].weaponLevel = 0;
    gSaveData.players[0].money = 100;
    gSaveData.players[1].chara = -1;
    gSaveData.players[2].chara = -1;
    gSaveData.players[3].chara = -1;
    gSaveData.costume[0][0] = 0;
    gSaveData.costume[1][0] = 0;
    gSaveData.costume[2][0] = 0;
    gSaveData.costume[3][0] = 0;
    if (joyGetButton(0, 0) & R_CBUTTONS) {
        joyDemoStartRecord(4);
    } else {
        joyDemoStartPlay(stageStaff0DemoInput, 0x142);
    }
    stageBuildFlagged2(1, 1, 100.0f, 20.0f, 0.0f, (u32) stageStaff0Placements, 0x8A, (u32) stageStaff0Placements2, 0x16);
    camAiSetFar(400);
    cambgCreate(gStageTask, 1, 800.0f);
    gaugeSetOn(0);
}
