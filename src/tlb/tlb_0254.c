#include "tlb.h"

/* File 254: "IMANISHI", a fixed-camera room (the stage list, file 13). */

extern ConnectData imanishiPlacesEne0FAParam[];
ConnectData imanishiPlacesEne0FAParam[] = {
    { 10.0f, 25, 0, 360.0f, 0.0f, -100.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef imanishiPlaces[] = {
    { 0, 10, 0, 80, ENE_CONNECT, 2, (u32) &imanishiPlacesEne0FAParam },
};

void imanishiInit(void) {
    stageBuildFixedCam(0x29, 0, 40.0f, 10.0f, -10.0f, (u32) imanishiPlaces, 1, 100.0f, 0.0f, 40.0f);
    camAiSetFar(1000);
}
