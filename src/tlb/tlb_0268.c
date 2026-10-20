#include "tlb.h"

extern ConnectData yorozuyaPlacesEne0FAParam0[];
extern ConnectData yorozuyaPlacesEne0FAParam1[];
ConnectData yorozuyaPlacesEne0FAParam0[] = {
    { 15.0f, 25, 0, -1115.0f, 0.0f, -125.0f, 0.0f, 0x0000, 0, 0 },
};
ConnectData yorozuyaPlacesEne0FAParam1[] = {
    { 15.0f, 25, 0, -1175.0f, 0.0f, -125.0f, 0.0f, 0x0000, 0, 0 },
};
EneDef yorozuyaPlaces[] = {
    { 0, 45, 0, 105, ENE_CONNECT, 2, (u32) &yorozuyaPlacesEne0FAParam0 },
    { 0, -15, 0, 105, ENE_CONNECT, 2, (u32) &yorozuyaPlacesEne0FAParam1 },
};

void yorozuyaInit(void) {
    stageBuildFixedCam(0xF, 0, 0.0f, 0.0f, 0.0f, (u32) yorozuyaPlaces, 2, 95.0f, 0.0f, 50.0f);
    camAiSetFar(1000);
}
