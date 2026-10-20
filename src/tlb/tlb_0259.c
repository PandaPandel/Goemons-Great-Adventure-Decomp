#include "tlb.h"

/* File 259: "ORACLE", a fixed-camera room (the stage list, file 13). */

extern ConnectData oraclePlacesEne0FAParam[];
extern PersonData oraclePlacesEne128Param[];
ConnectData oraclePlacesEne0FAParam[] = {
    { 10.0f, 25, 0, 360.0f, 0.0f, -100.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData oraclePlacesEne128Param[] = {
    { 736, 0x00000000, { 5, 8 }, 2, 0.0f, 0 },
};
EneDef oraclePlaces[] = {
    { 0, -40, 10, 135, ENE_CONNECT, 2, (u32) &oraclePlacesEne0FAParam },
    { 0, 0, 10, 135, ENE_CONNECT, 2, (u32) &oraclePlacesEne0FAParam },
    { 0, 40, 10, 135, ENE_CONNECT, 2, (u32) &oraclePlacesEne0FAParam },
    { 0, 0, -1, -80, ENE_SHOP_ROOM_BG, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_PURA, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_DANCER, 0, 0 },
    { 0, 0, 0, 0, ENE_SHOP_DANCER, 0, 0 },
    { 0, 0, 0, 0, ENE_URANAI_DECO, 0, 0 },
    { 0, 0, 0, 0, ENE_URANAI_CAMERA, 0, 0 },
    { 0, 40, 5, 0, ENE_URANAI_TABLE, 2, (u32) &oraclePlacesEne128Param },
};

void oracleInit(void) {
    stageBuildFixedCam(0x2E, 0, 40.0f, 10.0f, -10.0f, (u32) oraclePlaces, 0xA, -180.0f, 20.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(10.0f, 0.0f, 0.471238941f);
}
