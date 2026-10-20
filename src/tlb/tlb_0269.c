#include "tlb.h"

extern ConnectData yadoyaFrontPlacesEne0FAParam[];
ConnectData yadoyaFrontPlacesEne0FAParam[] = {
    { 10.0f, 25, 0, -360.0f, 0.0f, -140.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef yadoyaFrontPlaces[] = {
    { 0, 60, 0, 70, ENE_CONNECT, 2, (u32) &yadoyaFrontPlacesEne0FAParam },
};

void yadoyaFrontInit(void) {
    stageBuildFixedCam(0x10, 0, 0.0f, 0.0f, 0.0f, (u32) yadoyaFrontPlaces, 1, 100.0f, 0.0f, 0.0f);
    camAiSetFar(1000);
}
