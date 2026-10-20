#include "tlb.h"

extern ConnectData gannsoPlacesEne0FAParam[];
ConnectData gannsoPlacesEne0FAParam[] = {
    { 15.0f, 25, 0, -965.0f, 0.0f, -120.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef gannsoPlaces[] = {
    { 0, 40, 0, 105, ENE_CONNECT, 2, (u32) &gannsoPlacesEne0FAParam },
    { 0, 0, 0, 0, ENE_TABLE_GANSO, 0, 0 },
};

void gannsoInit(void) {
    stageBuildFixedCam(0xE, 0, 0.0f, 0.0f, 0.0f, (u32) gannsoPlaces, 2, 70.0f, 0.0f, -55.0f);
    camAiSetFar(1000);
    eneMakeEx(NULL, ENE_TABLE_GANSO, 0, 0.0f, 0.0f, 0.0f, 0, NULL);
}
