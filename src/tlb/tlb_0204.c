#include "tlb.h"

extern u8 stg0204BuildColl[];
extern u8 stg0204BuildCourse0[];
extern u8 stg0204BuildCourse1[];
EneDef stg0204Places[] = {
    { 0, 0, 0, 0, ENE_NOBUKO, 0, 0 },
    { 0, 0, 0, 0, ENE_DAI, 0, 0 },
    { 0, 0, 0, 0, ENE_IHEAD, 0, 0 },
    { 0, 0, 0, 0, ENE_SIPPO, 0, 0 },
    { 0, 0, 0, 0, ENE_BOMB, 0, 0 },
    { 0, 0, 0, 0, ENE_FIRE, 0, 0 },
    { 0, 0, 0, 0, ENE_IHAND, 0, 0 },
    { 0, 0, 0, 0, ENE_IHANDPOINT, 0, 0 },
    { 0, 0, 0, 0, ENE_ATOYA, 0, 0 },
    { 0, 0, 0, 0, ENE_BIG_BOMB, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_3, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_GOLDNEKO_MAIN_FLAGS_3, 0, 0 },
};

/* Stage 0x204's build. Its twin is tlb_0222's stageUnlisted222Build. */
void stg0204Build(void) {
    s32 pad; /* FAKE: the game's frame words */
    f32 side;
    f32 x;
    s32 pad2[2]; /* FAKE */
    s16 file;

    floadLoadSet(0x24);
    floadLoadSet(0x25);
    vramCreate(gStageTask);
    file = floadGetSetFile(0x25, 0);
    vramLoadColl((u32) stg0204BuildColl, file);
    PRINTF("当たりセット OK \n");
    vramSetCourse(1, 0x31, (Vec3f *) floadSegToRam((u32) stg0204BuildCourse0, 0x25, 0));
    vramSetCourse(2, 0x31, (Vec3f *) floadSegToRam((u32) stg0204BuildCourse1, 0x25, 0));
    PRINTF("コースセット OK \n");
    stageBuild3Places(2, 100.0f, 200.0f, 0.0f, 2, 107.0f, 200.0f, 5.0f, 2, 93.0f, 200.0f, -5.0f, (u32) stg0204Places, 0xC);
    PRINTF("ステージセット OK \n");
    camAiSetFar(600);
    x = vramNearestPlaceXZ(1, 0.0f, 0.0f, &side);
    eneMake(ENE_NOBUKO, 1, x, 165.0f, side);
    fadeCreate(gEffectTask);
    fadeSetLetterbox(0x140, 0x78, 0, 0x78);
    camSunDelete();
    gTimeOfDay = 23 << 16;
    gGameState.fade = 1;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
    gGameState.fadeStart = 0x30;
    gGameState.fadeTo[0] = 0xA0;
    gGameState.fadeTo[1] = 0x10;
    gGameState.fadeTo[2] = 0x10;
    gGameState.fadeEnd = 0x80;
    sndPlay(1);
    sndPlay(0x10D);
}
