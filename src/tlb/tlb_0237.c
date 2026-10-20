#include "tlb.h"

extern u8 stageSet128BuildColl[];
extern u8 stageSet128BuildCourse[];
extern ExitData stageSet128PlacesEne0F9Param[];
extern PersonData stageSet128Kind14DData[];
ExitData stageSet128PlacesEne0F9Param[] = {
    { 70.0f, 2, 5, -1, 0 },
};
PersonData stageSet128Kind14DData[] = {
    { 0, 0x00000000, { 15, 70 }, 0, 0.0f, 0 },
};
EneDef stageSet128Places[] = {
    { 0, 0, 0, 0, ENE_SEKISYO3_VRAM, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_SEKISYO3, 2, (u32) &stageSet128Kind14DData },
    { 0, 0, 0, 0, ENE_SEKISYO3_YAKUNIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SEKISYO3_TOBIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_SEKISYO3_CAMERA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_STOPPER, 0, 0 },
    { 0, 0, 0, 0, ENE_SEKISYO3_CHIUMI, 0, 0 },
    { 1, 84, -76, 0, ENE_EXIT_ESCAPE, 2, (u32) &stageSet128PlacesEne0F9Param },
};

void stageSet128Build(void) {
    floadLoadSet(0x80);
    floadLoadSet(0x75);
    vramCreate(gStageTask);
    /* FAKE: called as returning s32, as in tlb_0199: tlb.h has the definition's s16, and a
     * nested call with a narrow result gets a temp word of its own (frame 0x60 for 0x58). */
    vramLoadColl((u32) stageSet128BuildColl, ((s32 (*)(s32, s32)) floadGetSetFile)(0x80, 0));
    PRINTF("当たりセット OK \n");
    vramSetCourse(1, 0x31, (Vec3f *) floadSegToRam((u32) stageSet128BuildCourse, 0x80, 0));
    PRINTF("コースセット OK \n");
    stageBuild3Places(1, 249.0f, -50.0f, 0.0f, 1, 249.0f, -50.0f, 5.0f, 1, 249.0f, -50.0f, -5.0f, (u32) stageSet128Places, 8);
    PRINTF("ステージセット OK \n");
    camAiSnap();
    camAiSetFar(2000);
    cambgCreate(gStageTask, 4, 800.0f);
    eneMake(ENE_SEKISYO3_VRAM, 0, 0.0f, 0.0f, 0.0f);
    eneMakeEx(NULL, ENE_DEMO_SEKISYO3, 1, 433.0f, -59.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) stageSet128Kind14DData);
    eneMakeEx(NULL, ENE_SEKISYO3_CHIUMI, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
    sndPlay(0x9E);
}
