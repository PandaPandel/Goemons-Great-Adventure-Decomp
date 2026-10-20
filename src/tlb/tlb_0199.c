#include "tlb.h"

extern u8 stg0199BuildColl[];
extern u8 stg0199BuildCourse[];
extern ExitData stg0199PlacesEne0F9Param[];
extern PersonData stg0199Ene142Data[];
ExitData stg0199PlacesEne0F9Param[] = {
    { 70.0f, 0, 5, -1, 0 },
};
PersonData stg0199Ene142Data[] = {
    { 0, 0x00000000, { 15, 70 }, 0, 0.0f, 0 },
};
EneDef stg0199Places[] = {
    { 0, 0, 0, 0, ENE_ETSK_SEKISYO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_SEKISYO1, 2, (u32) &stg0199Ene142Data },
    { 0, 0, 0, 0, ENE_SEKISYO1_YAKUNIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SEKISYO1_TOBIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_SEKISYO1_CAMERA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_STOPPER, 0, 0 },
    { 1, 40, -65, 0, ENE_EXIT_ESCAPE, 2, (u32) &stg0199PlacesEne0F9Param },
};

void stg0199Build(void) {
    floadLoadSet(0x7E);
    floadLoadSet(0x75);
    vramCreate(gStageTask);
    /* FAKE: called as returning s32. tlb.h has the definition's s16, and a nested call with a
     * narrow result gets a temp word of its own: frame 0x58 for the game's 0x50. The file's
     * own prototype must have said int. */
    vramLoadColl((u32) stg0199BuildColl, ((s32 (*)(s32, s32)) floadGetSetFile)(0x7E, 0));
    PRINTF("当たりセット OK \n");
    vramSetCourse(1, 0x31, (Vec3f *) floadSegToRam((u32) stg0199BuildCourse, 0x7E, 0));
    PRINTF("コースセット OK \n");
    stageBuild3Places(1, 125.0f, -64.0f, 0.0f, 1, 125.0f, -64.0f, 5.0f, 1, 125.0f, -64.0f, -5.0f, (u32) stg0199Places, 7);
    PRINTF("ステージセット OK \n");
    camAiSnap();
    camAiSetFar(2000);
    cambgCreate(gStageTask, 1, 800.0f);
    eneMake(ENE_ETSK_SEKISYO_MAIN, 0, 0.0f, 0.0f, 0.0f);
    eneMakeEx(NULL, ENE_DEMO_SEKISYO1, 1, 419.0f, -58.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) stg0199Ene142Data);
    camSunSetTunes4(0x9C, 0x9A, 0x9B, 0x9C);
}
