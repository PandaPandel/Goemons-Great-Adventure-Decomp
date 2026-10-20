#include "tlb.h"

/* File 262: "HIDDEN WELL", a fixed-camera room (the stage list, file 13). */

extern ConnectData hiddenWellPlacesEne0FAParam[];
ConnectData hiddenWellPlacesEne0FAParam[] = {
    { 10.0f, 25, 0, 360.0f, 0.0f, -100.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef hiddenWellPlaces[] = {
    { 0, 10, 0, 80, ENE_CONNECT, 2, (u32) &hiddenWellPlacesEne0FAParam },
    { 0, 9, 0, 55, ENE_TOKAGE, 0, 0 },
};

void hiddenWellInit(void) {
    stageBuildFixedCam(0x44, 0, 40.0f, 10.0f, -10.0f, (u32) hiddenWellPlaces, 2, 100.0f, 0.0f, 40.0f);
    camAiSetFar(1000);
}
