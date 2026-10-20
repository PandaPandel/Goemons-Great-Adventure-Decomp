#include "tlb.h"

/* File 261: "SARAYA", the plate shop, a fixed-camera room (the stage list, file 13). */

extern ConnectData sarayaPlacesEne0FAParam[];
ConnectData sarayaPlacesEne0FAParam[] = {
    { 10.0f, 25, 0, 360.0f, 0.0f, -100.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef sarayaPlaces[] = {
    { 0, 10, 0, 80, ENE_CONNECT, 2, (u32) &sarayaPlacesEne0FAParam },
};

void sarayaInit(void) {
    stageBuildFixedCam(0x43, 0, 40.0f, 10.0f, -10.0f, (u32) sarayaPlaces, 1, 100.0f, 0.0f, 40.0f);
    camAiSetFar(1000);
}
