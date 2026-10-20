#include "tlb.h"

/* File 255: "NOUKA", the farmhouse, a fixed-camera room (the stage list, file 13). */

extern ConnectData noukaPlacesEne0FAParam[];
ConnectData noukaPlacesEne0FAParam[] = {
    { 10.0f, 25, 0, 360.0f, 0.0f, -100.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef noukaPlaces[] = {
    { 0, 10, 0, 80, ENE_CONNECT, 2, (u32) &noukaPlacesEne0FAParam },
};

void noukaInit(void) {
    stageBuildFixedCam(0x2A, 0, 40.0f, 10.0f, -10.0f, (u32) noukaPlaces, 1, 100.0f, 0.0f, 40.0f);
    camAiSetFar(1000);
}
