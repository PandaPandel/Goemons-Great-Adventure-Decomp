#include "tlb.h"

/* File 257: "GOEMON HOUSE", a fixed-camera room (the stage list, file 13). */

extern ConnectData goemonHousePlacesEne0FAParam[];
ConnectData goemonHousePlacesEne0FAParam[] = {
    { 10.0f, 25, 0, 360.0f, 0.0f, -100.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef goemonHousePlaces[] = {
    { 0, 10, 0, 80, ENE_CONNECT, 2, (u32) &goemonHousePlacesEne0FAParam },
};

void goemonHouseInit(void) {
    stageBuildFixedCam(0x2B, 0, 40.0f, 10.0f, -10.0f, (u32) goemonHousePlaces, 1, 100.0f, 0.0f, 40.0f);
    camAiSetFar(1000);
}
