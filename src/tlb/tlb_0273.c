#include "tlb.h"

EneDef yadoyaUmePlaces[] = {
    { 0, 0, 0, 0, ENE_SHOP_UME_G, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_UME_E, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_UME_S, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_UME_Y, 0, 0 },
    { 0, 80, 2, 25, ENE_SHOP_UME_P, 0, 0 },
    { 0, 69, 2, -82, ENE_SHOP_UME_O, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_CAMERA, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_YUGE, 0, 0 },
    { 0, 26, 18, -28, ENE_ROOM_YADOYA_STEAM, 0, 0 },
};

void yadoyaUmeInit(void) {
    stageBuildFixedCam(0x27, 0, 40.0f, 10.0f, -10.0f, (u32) yadoyaUmePlaces, 0xB, 100.0f, 0.0f, 40.0f);
    camAiSetFar(1000);
    eneMakeEx(NULL, ENE_ROOM_YADOYA, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
}
