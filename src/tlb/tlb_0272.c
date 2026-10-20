#include "tlb.h"

EneDef yadoyaTakePlaces[] = {
    { 0, 0, 0, 0, ENE_SHOP_TAK_G, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_TAK_E, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_TAK_S, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_TAK_Y, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_CAMERA, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, 5, 18, -21, ENE_ROOM_YADOYA_STEAM, 0, 0 },
};

void yadoyaTakeInit(void) {
    stageBuildFixedCam(0x26, 0, 40.0f, 10.0f, -10.0f, (u32) yadoyaTakePlaces, 9, 100.0f, 0.0f, 40.0f);
    camAiSetFar(1000);
    eneMakeEx(NULL, ENE_ROOM_YADOYA, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
}
