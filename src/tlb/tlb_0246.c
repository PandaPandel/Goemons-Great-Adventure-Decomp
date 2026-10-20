#include "tlb.h"

extern u8 sekisyo4InitColl[];
extern u8 sekisyo4InitCourse[];
extern ExitData sekisyo4PlacesEne0F9Param[];
extern PersonData sekisyo4Ene153Arg[];
ExitData sekisyo4PlacesEne0F9Param[] = {
    { 70.0f, 3, 4, 4, 0 },
};
PersonData sekisyo4Ene153Arg[] = {
    { 0, 0x00000000, { 15, 70 }, 0, 0.0f, 0 },
};
EneDef sekisyo4Places[] = {
    { 0, 0, 0, 0, ENE_SEKISYO4_VRAM, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_SEKISYO4, 2, (u32) &sekisyo4Ene153Arg },
    { 0, 0, 0, 0, ENE_SEKISYO4_YAKUNIN, 0, 0 },
    { 0, 0, 0, 0, ENE_SEKISYO4_TOBIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_SEKISYO4_CAMERA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_STOPPER, 0, 0 },
    { 0, 0, 0, 0, ENE_SEKISYO4_IZUMI, 0, 0 },
    { 1, 220, -70, 0, ENE_EXIT_ESCAPE, 2, (u32) &sekisyo4PlacesEne0F9Param },
};

void sekisyo4Init(void) {
    floadLoadSet(0xDA);
    floadLoadSet(0x75);
    vramCreate(gStageTask);
    /* FAKE: called as returning s32, as in tlb_0199 (the definition's s16 result gets a temp
     * word of its own: frame 0x60 for the game's 0x58). */
    vramLoadColl((u32) sekisyo4InitColl, ((s32 (*)(s32, s32)) floadGetSetFile)(0xDA, 0));
    PRINTF("当たりセット OK \n");
    vramSetCourse(1, 0x31, (Vec3f *) floadSegToRam((u32) sekisyo4InitCourse, 0xDA, 0));
    PRINTF("コースセット OK \n");
    stageBuild3Places(1, 328.0f, -75.0f, 0.0f, 1, 328.0f, -75.0f, 5.0f, 1, 328.0f, -75.0f, -5.0f, (u32) sekisyo4Places, 8);
    PRINTF("ステージセット OK \n");
    camAiSnap();
    eneMake(ENE_SEKISYO4_VRAM, 0, 0.0f, 0.0f, 0.0f);
    eneMakeEx(NULL, ENE_DEMO_SEKISYO4, 1, 585.0f, -55.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) sekisyo4Ene153Arg);
    eneMakeEx(NULL, ENE_SEKISYO4_IZUMI, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
    camAiSetFar(600);
    camSunSetProgram(6);
    sndPlay(0x9F);
}
