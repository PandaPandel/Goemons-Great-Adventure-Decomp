#include "tlb.h"

#define DPRINTF

extern u8 stageUnlisted222BuildColl[];
extern u8 stageUnlisted222BuildCourse0[];
extern u8 stageUnlisted222BuildCourse1[];
EneDef stage222Places[] = {
    { 0, 0, 0, 0, ENE_SANBA_MAIN, 0, 0 },
};

void stageUnlisted222Build(void) {
    f32 x;
    f32 z;

    floadLoadSet(0xFA);
    floadLoadSet(0x26);
    floadLoadSet(0x24);
    floadLoadSet(0x25);
    vramCreate(gStageTask);
    /* FAKE: called as returning s32, as in tlb_0199. tlb.h has the definition's s16, and a
     * nested call with a narrow result gets a temp word of its own (the spill one word low).
     * The file's own prototype must have said int. */
    vramLoadColl((u32) stageUnlisted222BuildColl, ((s32 (*)(s32, s32)) floadGetSetFile)(0x25, 0));
    DPRINTF("当たりセット OK \n");
    vramSetCourse(1, 0x31, (Vec3f *) floadSegToRam((u32) stageUnlisted222BuildCourse0, 0x25, 0));
    vramSetCourse(2, 0x31, (Vec3f *) floadSegToRam((u32) stageUnlisted222BuildCourse1, 0x25, 0));
    DPRINTF("コースセット OK \n");
    stageBuild3Places(2, 100.0f, 200.0f, 0.0f, 2, 107.0f, 200.0f, 0.0f, 2, 107.0f, 200.0f, 0.0f, (u32) stage222Places, 1);
    DPRINTF("ステージセット OK \n");
    camAiSetFar(600);
    x = vramNearestPlaceXZ(1, 0.0f, 0.0f, &z);
    eneMake(ENE_SANBA_MAIN, 1, x, 0.0f, z);
    DPRINTF("ボスセット OK \n");
    fadeCreate(gEffectTask);
    fadeSetLetterbox(320, 40, 0, 200);
    camSunDelete();
    gTimeOfDay = 12 << 16;
    gGameState.fade = 1;
    gGameState.fadeFrom.r = 0x80;
    gGameState.fadeFrom.g = 0x80;
    gGameState.fadeFrom.b = 0xFF;
    gGameState.fadeStart = 0x40;
    gGameState.fadeTo[0] = 0xFF;
    gGameState.fadeTo[1] = 0xFF;
    gGameState.fadeTo[2] = 0xFF;
    gGameState.fadeEnd = 0xA0;
}
