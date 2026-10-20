#include "tlb.h"

EneDef stage4BossPlaces[] = {
    { 0, 0, 0, 0, ENE_KABUKI_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_KABUKI, 0, 0 },
    { 0, 0, 0, 0, ENE_KABUKI_UMIMONK, 0, 0 },
    { 0, 0, 0, 0, ENE_KABUKI_DOOR, 0, 0 },
    { 0, 0, 0, 0, ENE_KABUKI_KIRI, 0, 0 },
    { 0, 0, 0, 0, ENE_KABUKI_KAGE, 0, 0 },
    { 0, 0, 0, 0, ENE_KABUKI_WATER, 0, 0 },
    { 0, 0, 0, 0, ENE_KABUKI_HASIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_TANU_WALL_FLAGS_3, 0, 0 },
    { 0, 0, 0, 0, ENE_KABUKI_ATKPOINT, 0, 0 },
    { 0, 0, 0, 0, ENE_BIG_BOMB, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_3, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_GOLDNEKO_MAIN_FLAGS_3, 0, 0 },
};

void stage4BossInit(void) {
    Camera *cam;
    f32 side;
    s32 pad[2];
    s32 i;

    stageBuildCamMode(0x4E, 0, 1, 0.5f, 48.0f, 0.0f, (u32) stage4BossPlaces, 0xD);
    camAiSetFar(1500);
    vramNearestPlaceXZ(1, 0.0f, 0.0f, &side);
    eneMake(ENE_KABUKI_MAIN, 0, 0.0f, 0.0f, 0.0f);
    fadeCreate(gEffectTask);
    fadeSetLetterbox(320, 120, 0, 120);
    camAiSetHold(1);
    cam = camAiGetCam();
    camSetAt(cam, -291.0f, 72.0f, -168.0f);
    camSetEye(cam, -237.0f, 80.0f, -18.0f);
    for (i = 0; i < 4; i++) {
        if (plIsInPlay(i)) {
            plSetAction(gGameWork.players[i], 0xD7);
        }
    }
    camSunSetClockStop(0);
    gGameWork.time = 0;
    sndPlay(0x520);
}
