#include "tlb.h"

EneDef stage84Places[] = {
    { 0, 0, 0, 0, ENE_ONI_FIGHT_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ONI_FIGHT_SPIRIT, 0, 0 },
    { 0, 0, 0, 0, ENE_ONI_FIGHT_ONI, 0, 0 },
    { 0, 0, 0, 0, ENE_ONI_FIGHT_DOOR, 0, 0 },
    { 0, 0, 0, 0, ENE_ONI_FIGHT_PILLAR, 0, 0 },
    { 0, 0, 0, 0, ENE_ONI_FIGHT_HAHEN, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_WALL_FLAGS_3, 0, 0 },
    { 0, 0, 0, 0, ENE_BIG_BOMB, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_3, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_GOLDNEKO_MAIN_FLAGS_3, 0, 0 },
};

void stage84DemoBuild(void) {
    s32 pad; /* the game's frame: 0x60, side at 0x58 */
    f32 side;
    Camera *cam;
    s32 i;

    stageBuildCamMode(0x54, 0, 1, 60.0f, 0.0f, 0.0f, (u32) stage84Places, 0xA);
    camAiSetFar(600);
    camAiAddLimit(1, -1, 0xAB);
    camAiAddLimit(1, 1, 0x23F);
    vramNearestPlaceXZ(1, 0.0f, 0.0f, &side);
    eneMake(ENE_ONI_FIGHT_MAIN, 0, 0.0f, 0.0f, 0.0f);
    fadeCreate(gEffectTask);
    fadeSetLetterbox(320, 120, 0, 120);
    camAiSetHold(1);
    cam = camAiGetCam();
    camSetAt(cam, -139.0f, 6.0f, 51.0f);
    camSetEye(cam, -80.0f, 5.0f, 51.0f);
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plSetAction(gGameWork.players[i], 0xD7);
        }
    }
    camSunSetClockStop(0);
    gGameWork.time = 0;
    camSunSetSkyOn(0);
    gGameState.fade = 1;
    gGameState.fadeFrom.r = 0;
    gGameState.fadeFrom.g = 0;
    gGameState.fadeFrom.b = 0;
    gGameState.fadeStart = 0x80;
    gGameState.fadeTo[0] = 0;
    gGameState.fadeTo[1] = 0;
    gGameState.fadeTo[2] = 0;
    gGameState.fadeEnd = 0x100;
    sndPlay(0x520);
}
