#include "tlb.h"

extern u8 stageSet141BuildColl[];
extern ConnectData stageSet141PlacesEne0FAParam[];
extern PersonData stageSet141PlacesEne179Param[];
extern PersonData stageSet141Kind17BData[];
ConnectData stageSet141PlacesEne0FAParam[] = {
    { 10.0f, 71, 0, 102.0f, 140.0f, -3.0f, 0.0f, 0x000A, 0, 0 },
};
PersonData stageSet141Kind17BData[] = {
    { 600, 0x00000000, { 4, 8 }, 0, 0.0f, 0 },
};
PersonData stageSet141PlacesEne179Param[] = {
    { 552, 0x00000000, { 4, 8 }, 0, 0.0f, 0 },
};
EneDef stageSet141Places[] = {
    { 0, 0, 0, 0, ENE_DEMO_HANABI, 2, (u32) &stageSet141Kind17BData },
    { 0, 0, 0, 0, ENE_HANADESHI, 2, (u32) &stageSet141Kind17BData },
    { 0, 0, 0, 0, ENE_HANAEXIT, 0, 0 },
    { 0, 0, 0, 0, ENE_HANABI_HIROBA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_HANAOYA, 2, (u32) &stageSet141PlacesEne179Param },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 2, 0, 101, ENE_CONNECT, 2, (u32) &stageSet141PlacesEne0FAParam },
    { 0, 0, 0, 0, ENE_HIDUTU, 0, 0 },
    { 0, 0, 0, 0, ENE_ITEM_ATARI, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_JIZOU_FILE_138, 0, 0 },
};

void stageSet141Build(void) {
    floadLoadSet(0x8D);
    floadLoadSet(0x121);
    vramCreate(gStageTask);
    vramLoadColl((u32) stageSet141BuildColl, floadGetSetFile(0x8D, 0));
    PRINTF("当たりセット OK \n");
    stageBuildPlain(0, 3.0f, 0.0f, 66.0f, (u32) stageSet141Places, 0xC);
    PRINTF("ステージセット OK \n");
    camAiSetFar(1000);
    cambgCreate(gStageTask, 1, 800.0f);
    eneMake(ENE_HANABI_HIROBA_MAIN, 0, 0.0f, 0.0f, 0.0f);
    sndPlay(0xBA);
    eneMakeEx(NULL, ENE_DEMO_HANABI, 0, 0.0f, 0.0f, 0.0f, 2, (u32 (*)(void *, void *, void *)) stageSet141Kind17BData);
    eneMakeEx(NULL, ENE_HANAEXIT, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
}
