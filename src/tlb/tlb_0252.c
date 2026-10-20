#include "tlb.h"

/* File 252: the fifth stage's boss, "5ST BOSS-1" and "5ST BOSS-2" (the stage list, file 13). */

extern u8 boss5Coll[];
extern u8 boss5Course[];
EneDef boss5Places1[] = {
    { 0, 0, 0, 0, ENE_DOCHUKI_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_DOCHUKI, 0, 0 },
    { 0, 0, 0, 0, ENE_DOCHUKI_BEHIMOS, 0, 0 },
    { 0, 0, 0, 0, ENE_DOCHUKI_VRAM1, 0, 0 },
    { 0, 0, 0, 0, ENE_DOCHUKI_VRAM2, 0, 0 },
    { 0, 0, 0, 0, ENE_DOCHUKI_CHINKO, 0, 0 },
    { 0, 0, 0, 0, ENE_DOCHUKI_GENKI, 0, 0 },
    { 0, 0, 0, 0, ENE_DOCHUKI_NTAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_DOCHUKI_EFFECT, 0, 0 },
    { 0, 0, 0, 0, ENE_DOCHUKI_BEAM, 0, 0 },
    { 0, 0, 0, 0, ENE_DOCHUKI_CORE, 0, 0 },
    { 0, 0, 0, 0, ENE_DOCHUKI_IKI, 0, 0 },
    { 0, 0, 0, 0, ENE_DOCHUKI_BAT, 0, 0 },
    { 0, 0, 0, 0, ENE_DOCHUKI_HANA, 0, 0 },
    { 0, 0, 0, 0, ENE_BIG_BOMB, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_3, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_GOLDNEKO_MAIN_FLAGS_3, 0, 0 },
};
EneDef boss5Places2[] = {
    { 0, 0, 0, 0, ENE_DOCHUKI_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_DOCHUKI, 0, 0 },
};

void boss5Init1(void) {
    s32 pad[2];
    Obj *bg;
    Camera *cam;
    s32 i;

    floadLoadSet(0x2B);
    vramCreate(gStageTask);
    vramLoadColl((u32) boss5Coll, floadGetSetFile(0x2B, 0));
    PRINTF("当たりセット OK \n");
    vramSetCourse(1, 0x31, (Vec3f *) floadSegToRam((u32) boss5Course, 0x2B, 0));
    PRINTF("コースセット OK \n");
    stageBuildPlain(1, 342.0f, 250.0f, 0.0f, (u32) boss5Places1, 0x11);
    camAiSetFar(600);
    bg = cambgCreate(gStageTask, 6, 800.0f);
    cambgMove(bg, 0, 0);
    cambgScrollY(bg, 0);
    cambgSetAlpha(bg, 0x40);
    eneMake(ENE_DOCHUKI_MAIN, 0, 0.0f, 0.0f, 0.0f);
    fadeCreate(gEffectTask);
    fadeSetLetterbox(0x140, 0x78, 0, 0x78);
    camAiSetHold(1);
    cam = camAiGetCam();
    camSetAt(cam, 64.0f, 160.0f, 97.0f);
    camSetEye(cam, 135.0f, 130.0f, 135.0f);
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plSetAction(gGameWork.players[i], 0xD7);
        }
    }
    camSunSetClockStop(0);
    camSunSetSkyOn(0);
    camSunSetProgram(0xE);
    gGameState.fade = 0;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
    gGameState.fadeStart = 0x30;
    gGameState.fadeTo[0] = 0x40;
    gGameState.fadeTo[1] = 0x40;
    gGameState.fadeTo[2] = 0x40;
    gGameState.fadeEnd = 0x80;
    camSunSetFogColor(16, 16, 16);
    camSunSetColor(0xD0, 0xD0, 0xFF);
}

/* The game's frame has a word above side and two below it (side at 0x40, the FileSet temp at
 * 0x30): the pads. */
void boss5Init2(void) {
    s32 pad;
    f32 side;
    s32 pad2[2];

    floadLoadSet(0x2B);
    vramCreate(gStageTask);
    vramLoadColl((u32) boss5Coll, floadGetSetFile(0x2B, 0));
    PRINTF("当たりセット OK \n");
    vramSetCourse(1, 0x31, (Vec3f *) floadSegToRam((u32) boss5Course, 0x2B, 0));
    PRINTF("コースセット OK \n");
    //! @bug 0x11 is boss5Places1's count; boss5Places2 has 2. eneMakeBench reads on into the
    //! file's .rodata, the strings above: entries 2-4 have negative kinds (bytes of the first
    //! three strings), each counted at used[kind], before the table; entry 5's kind is 0x0A00
    //! (the third string's "\n"), over 1000, so it gives up and the kind table is never made.
    stageBuildPlain(1, 100.0f, 69.0f, 0.0f, (u32) boss5Places2, 0x11);
    camAiSetFar(800);
    cambgCreate(gStageTask, 6, 800.0f);
    vramNearestPlaceXZ(1, 0.0f, 0.0f, &side);
    eneMake(ENE_DOCHUKI_MAIN, 0, 0.0f, 0.0f, 0.0f);
    fadeCreate(gEffectTask);
    fadeSetLetterbox(0x140, 0x78, 0, 0x78);
}
