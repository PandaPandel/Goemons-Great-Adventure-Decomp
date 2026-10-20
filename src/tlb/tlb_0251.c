#include "tlb.h"

/* File 251: "SEKISYO 5", the fifth checkpoint (the stage list, file 13). */

extern u8 sekisyo5InitColl[];
extern u8 sekisyo5InitCourse[];
extern ExitData sekisyo5PlacesEne0F9Param[];
extern PersonData sekisyo5KindData[];
ExitData sekisyo5PlacesEne0F9Param[] = {
    { 70.0f, 4, 4, 3, 0 },
};
PersonData sekisyo5KindData[] = {
    { 0, 0x00000000, { 15, 70 }, 0, 0.0f, 0 },
};
EneDef sekisyo5Places[] = {
    { 0, 0, 0, 0, ENE_SEKISYO5_VRAM, 0, 0 },
    { 0, 0, 0, 0, ENE_D_SEKISYO5, 2, (u32) &sekisyo5KindData },
    { 0, 0, 0, 0, ENE_SEKISYO5_YAKUNIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SEKISYO5_TOBIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_SEKISYO5_CAMERA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_STOPPER, 0, 0 },
    { 1, 35, -108, 0, ENE_EXIT_ESCAPE, 2, (u32) &sekisyo5PlacesEne0F9Param },
};

void sekisyo5Init(void) {
    floadLoadSet(0xDD);
    floadLoadSet(0x75);
    vramCreate(gStageTask);
    /* FAKE: called as returning s32, as in tlb_0199 (the definition's s16 result gets a temp
     * word of its own). */
    vramLoadColl((u32) sekisyo5InitColl, ((s32 (*)(s32, s32)) floadGetSetFile)(0xDD, 0));
    PRINTF("当たりセット OK \n");
    vramSetCourse(1, 0x31, (Vec3f *) floadSegToRam((u32) sekisyo5InitCourse, 0xDD, 0));
    PRINTF("コースセット OK \n");
    stageBuild3Places(1, 181.0f, -84.0f, 0.0f, 1, 181.0f, -84.0f, 5.0f, 1, 181.0f, -84.0f, -5.0f, (u32) sekisyo5Places, 7);
    PRINTF("ステージセット OK \n");
    camAiSnap();
    camAiSetFar(400);
    camSunSetProgram(7);
    camAiSetMode(3);
    eneMake(ENE_SEKISYO5_VRAM, 0, 0.0f, 0.0f, 0.0f);
    camAiSetFar(2000);
    eneMakeEx(NULL, ENE_D_SEKISYO5, 1, 418.0f, -64.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) sekisyo5KindData);
    camSunSetTunes4(0xA0, 0x9A, 0x9B, 0xA0);
}
