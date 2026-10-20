#include "tlb.h"

/* File 256: "NAGAYA", the row house, a fixed-camera room (the stage list, file 13). */

extern ConnectData nagayaPlacesEne0FAParam[];
ConnectData nagayaPlacesEne0FAParam[] = {
    { 10.0f, 25, 0, 360.0f, 0.0f, -100.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef nagayaPlaces[] = {
    { 0, 10, 0, 80, ENE_CONNECT, 2, (u32) &nagayaPlacesEne0FAParam },
};

void nagayaInit(void) {
    stageBuildFixedCam(0x2C, 0, 40.0f, 10.0f, -10.0f, (u32) nagayaPlaces, 1, 100.0f, 0.0f, 40.0f);
    camAiSetFar(1000);
}
