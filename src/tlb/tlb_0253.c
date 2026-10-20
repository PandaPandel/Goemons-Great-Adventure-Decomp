#include "tlb.h"

/* File 253: "MITSUYA", a fixed-camera room (the stage list, file 13). */

extern ConnectData mitsuyaPlacesEne0FAParam[];
ConnectData mitsuyaPlacesEne0FAParam[] = {
    { 10.0f, 25, 0, 360.0f, 0.0f, -100.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef mitsuyaPlaces[] = {
    { 0, 10, 0, 80, ENE_CONNECT, 2, (u32) &mitsuyaPlacesEne0FAParam },
};

void mitsuyaInit(void) {
    stageBuildFixedCam(0x28, 0, 40.0f, 10.0f, -10.0f, (u32) mitsuyaPlaces, 1, 100.0f, 0.0f, 40.0f);
    camAiSetFar(1000);
}
