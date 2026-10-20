#include "tlb.h"

extern u8 sekisyo2StageBuildColl[];
extern u8 sekisyo2StageBuildCourse[];
extern ExitData sekisyo2PlacesEne0F9Param[];
extern PersonData sekisyo2Ene147Arg[];
ExitData sekisyo2PlacesEne0F9Param[] = {
    { 70.0f, 1, 4, -1, 0 },
};
PersonData sekisyo2Ene147Arg[] = {
    { 0, 0x00000000, { 15, 70 }, 0, 0.0f, 0 },
};
EneDef sekisyo2Places[] = {
    { 0, 0, 0, 0, ENE_SEKISYO2_VRAM, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_SEKISYO2, 2, (u32) &sekisyo2Ene147Arg },
    { 0, 0, 0, 0, ENE_SEKISYO2_YAKUNIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SEKISYO2_TOBIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_SEKISYO2_CAMERA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_STOPPER, 0, 0 },
    { 0, 0, 0, 0, ENE_SEKISYO2_UMI, 0, 0 },
    { 1, 84, -76, 0, ENE_EXIT_ESCAPE, 2, (u32) &sekisyo2PlacesEne0F9Param },
};

/* One variable, p, holds the course's points and then the background. */
void sekisyo2StageBuild(void) {
    void *p;

    floadLoadSet(0x7F);
    floadLoadSet(0x75);
    vramCreate(gStageTask);
    vramLoadColl((u32) sekisyo2StageBuildColl, floadGetSetFile(0x7F, 0));
    PRINTF("当たりセット OK \n");
    p = (void *) floadSegToRam((u32) sekisyo2StageBuildCourse, 0x7F, 0);
    vramSetCourse(1, 0x31, p);
    PRINTF("コースセット OK \n");
    stageBuild3Places(1, 189.0f, -72.0f, 0.0f, 1, 189.0f, -72.0f, 5.0f, 1, 189.0f, -72.0f, -5.0f, (u32) sekisyo2Places, 8);
    PRINTF("ステージセット OK \n");
    camAiSnap();
    camAiSetFar(2000);
    p = cambgCreate(gStageTask, 3, 800.0f);
    cambgMove(p, 0, -8);
    cambgScrollY(p, 0);
    eneMake(ENE_SEKISYO2_VRAM, 0, 0.0f, 0.0f, 0.0f);
    eneMakeEx(NULL, ENE_DEMO_SEKISYO2, 1, 433.0f, -59.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) sekisyo2Ene147Arg);
    eneMakeEx(NULL, ENE_SEKISYO2_UMI, 0, 0.0f, 1.0f, 0.0f, 0, NULL);
    sndPlay(0x9D);
}
