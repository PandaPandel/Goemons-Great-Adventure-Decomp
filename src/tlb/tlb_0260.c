#include "tlb.h"

/* File 260: "POLICE BOX", a fixed-camera room (the stage list, file 13). */

extern ConnectData policeBoxPlacesEne0FAParam[];
ConnectData policeBoxPlacesEne0FAParam[] = {
    { 10.0f, 25, 0, 360.0f, 0.0f, -100.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef policeBoxPlaces[] = {
    { 0, 10, 0, 80, ENE_CONNECT, 2, (u32) &policeBoxPlacesEne0FAParam },
};

void policeBoxInit(void) {
    stageBuildFixedCam(0x42, 0, 5.0f, 0.0f, 28.0f, (u32) policeBoxPlaces, 1, 100.0f, 0.0f, 40.0f);
    camAiSetFar(1000);
}
