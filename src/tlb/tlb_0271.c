#include "tlb.h"

extern ConnectData honnkePlacesEne0FAParam[];
ConnectData honnkePlacesEne0FAParam[] = {
    { 10.0f, 25, 0, 360.0f, 0.0f, -100.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef honnkePlaces[] = {
    { 0, 10, 0, 80, ENE_CONNECT, 2, (u32) &honnkePlacesEne0FAParam },
};

void honnkeInit(void) {
    stageBuildFixedCam(0x20, 0, 40.0f, 10.0f, -10.0f, (u32) honnkePlaces, 1, 100.0f, 0.0f, 40.0f);
    camAiSetFar(1000);
}
