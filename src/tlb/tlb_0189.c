#include "tlb.h"

extern Tlb99Data stg0189PlacesEne0F3Param[];
u32 stg0189UnusedParam[] = { 0x42A00000, 0x00000000, 0x00000001, 0x00000001, 0x04000000 };
Tlb99Data stg0189PlacesEne0F3Param[] = {
    { 0, 1, { 0, -1, -1, 0 }, 152 },
};
EneDef stg0189Places[] = {
    { 0, 0, 0, 0, ENE_TANU, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_EFF, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_KOBAN, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR_MOJI, 0, 0 },
    { 0, 0, 0, 0, ENE_MES_STAGE_CLEAR, 0, 0 },
    { 1, 300, 30, 0, ENE_TANU_J, 2, (u32) &stg0189PlacesEne0F3Param },
    { 0, 0, 0, 0, ENE_EPRG_TANU, 0, 0 },
};

void stg0189Build(void) {
    stageBuildBasic(1, 1, 50.0f, 20.0f, 0.0f, (u32) stg0189Places, 7);
    camAiSetFar(600);
    cambgCreate(gStageTask, 1, 800.0f);
}
