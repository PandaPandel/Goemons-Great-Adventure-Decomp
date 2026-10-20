#include "tlb.h"

/* File 258: "YADOYA MATSU", the Matsu inn, a fixed-camera room (the stage list, file 13). */

EneDef yadoyaMatsuPlaces[] = {
    { 0, 30, 17, -25, ENE_SHOP_AWA, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MAT_G_RUN, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MAT_E, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MAT_S, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_MAT_Y, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA, 0, 0 },
    { 0, 0, 0, 0, ENE_ROOM_YADOYA_CAMERA, 0, 0 },
};

void yadoyaMatsuInit(void) {
    stageBuildFixedCam(0x2D, 0, 40.0f, 10.0f, -10.0f, (u32) yadoyaMatsuPlaces, 7, 100.0f, 0.0f, 40.0f);
    camAiSetFar(1000);
    eneMakeEx(NULL, ENE_ROOM_YADOYA, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
}
