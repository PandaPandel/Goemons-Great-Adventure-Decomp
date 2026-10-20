#include "tlb.h"

extern ConnectData stg44PlacesAEne0FAParam[];
extern PersonData stg44PlacesAEne196Param[];
extern ConnectData stg44PlacesBEne0FAParam[];
extern PersonData stg44PlacesBEne196Param[];
extern ConnectData stg44PlacesCEne0FAParam[];
extern PersonData stg44PlacesCEne196Param[];
extern ConnectData stg44PlacesDEne0FAParam[];
extern PersonData stg44PlacesDEne196Param[];
extern ConnectData stg44PlacesEEne0FAParam[];
extern PersonData stg44PlacesEEne1D9Param[];
extern ConnectData stg44PlacesFEne0FAParam[];
extern PersonData stg44PlacesFEne1D9Param[];
ConnectData stg44PlacesAEne0FAParam[] = {
    { 16.0f, 71, 0, -167.0f, 0.0f, 187.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stg44PlacesAEne196Param[] = {
    { 592, 0x00000000, { 7, 100 }, 2, 0.0f, 0 },
};
EneDef stg44PlacesA[] = {
    { 0, 0, 50, 0, ENE_CONNECT, 2, (u32) &stg44PlacesAEne0FAParam },
    { 0, -30, 12, 53, ENE_TOKAGE, 2, (u32) &stg44PlacesAEne196Param },
};
ConnectData stg44PlacesBEne0FAParam[] = {
    { 16.0f, 71, 0, 423.0f, -29.0f, 61.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stg44PlacesBEne196Param[] = {
    { 593, 0x00000000, { 7, 100 }, 2, 0.0f, 0 },
};
EneDef stg44PlacesB[] = {
    { 0, 0, 50, 0, ENE_CONNECT, 2, (u32) &stg44PlacesBEne0FAParam },
    { 0, -30, 12, 53, ENE_TOKAGE, 2, (u32) &stg44PlacesBEne196Param },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};
ConnectData stg44PlacesCEne0FAParam[] = {
    { 16.0f, 71, 0, 165.0f, -59.0f, 250.0f, 1.5707964f, 0x0000, 0, 0 },
};
PersonData stg44PlacesCEne196Param[] = {
    { 590, 0x00000000, { 7, 100 }, 2, 0.0f, 0 },
};
EneDef stg44PlacesC[] = {
    { 0, 0, 50, 0, ENE_CONNECT, 2, (u32) &stg44PlacesCEne0FAParam },
    { 0, -30, 12, 53, ENE_TOKAGE, 2, (u32) &stg44PlacesCEne196Param },
};
ConnectData stg44PlacesDEne0FAParam[] = {
    { 16.0f, 71, 0, 148.0f, 140.0f, -33.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stg44PlacesDEne196Param[] = {
    { 591, 0x00000000, { 7, 100 }, 2, 0.0f, 0 },
};
EneDef stg44PlacesD[] = {
    { 0, 0, 50, 0, ENE_CONNECT, 2, (u32) &stg44PlacesDEne0FAParam },
    { 0, -30, 12, 53, ENE_TOKAGE, 2, (u32) &stg44PlacesDEne196Param },
};
ConnectData stg44PlacesEEne0FAParam[] = {
    { 16.0f, 26, 0, 115.0f, 0.0f, 2.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData stg44PlacesEEne1D9Param[] = {
    { 495, 0x00000000, { 7, 100 }, 4, 0.0f, 0 },
};
EneDef stg44PlacesE[] = {
    { 0, 0, 50, 0, ENE_CONNECT, 2, (u32) &stg44PlacesEEne0FAParam },
    { 0, -30, 12, 53, ENE_TOKAGE_HAGU, 2, (u32) &stg44PlacesEEne1D9Param },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};
ConnectData stg44PlacesFEne0FAParam[] = {
    { 16.0f, 46, 6, 178.0f, 392.0f, 0.0f, 3.1415927f, 0x0000, 0, 0 },
};
PersonData stg44PlacesFEne1D9Param[] = {
    { 544, 0x00000000, { 7, 100 }, 4, 0.0f, 0 },
};
EneDef stg44PlacesF[] = {
    { 0, 0, 50, 0, ENE_CONNECT, 2, (u32) &stg44PlacesFEne0FAParam },
    { 0, -30, 12, 53, ENE_TOKAGE_HAGU, 2, (u32) &stg44PlacesFEne1D9Param },
    { 0, 0, 0, 0, ENE_DEMO_TEGATA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRA, 0, 0 },
    { 0, 0, 0, 0, ENE_DEMO_KIRAKIRA, 0, 0 },
};

void stg44BuildA(void) {
    stageBuildFixedCam(0x44, 0, 0.0f, 10.0f, 0.0f, (u32) stg44PlacesA, 2, 85.0f, 25.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(10.0f, 0.0f, 0.6283185482f);
    sndSetReverb(1, 0xF);
}

void stg44BuildB(void) {
    stageBuildFixedCam(0x44, 0, 0.0f, 10.0f, 0.0f, (u32) stg44PlacesB, 5, 85.0f, 25.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(10.0f, 0.0f, 0.6283185482f);
    sndSetReverb(1, 0xF);
}

void stg44BuildC(void) {
    stageBuildFixedCam(0x44, 0, 0.0f, 10.0f, 0.0f, (u32) stg44PlacesC, 2, 85.0f, 25.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(10.0f, 0.0f, 0.6283185482f);
    sndSetReverb(1, 0xF);
}

void stg44BuildD(void) {
    stageBuildFixedCam(0x44, 0, 0.0f, 10.0f, 0.0f, (u32) stg44PlacesD, 2, 85.0f, 25.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(10.0f, 0.0f, 0.6283185482f);
    sndSetReverb(1, 0xF);
}

void stg44BuildE(void) {
    stageBuildFixedCam(0x44, 0, 0.0f, 10.0f, 0.0f, (u32) stg44PlacesE, 5, 85.0f, 25.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(10.0f, 0.0f, 0.6283185482f);
    sndSetReverb(1, 0xF);
}

void stg44BuildF(void) {
    stageBuildFixedCam(0x44, 0, 0.0f, 10.0f, 0.0f, (u32) stg44PlacesF, 5, 85.0f, 25.0f, 0.0f);
    camAiSetFar(1000);
    camAiSetTurnLimit(10.0f, 0.0f, 0.6283185482f);
    sndSetReverb(1, 0xF);
    camSunAddSoundPair(0x28D, 0x520);
    camSunAddSoundPair(0x12F, 0x520);
    sndPlay(0x812A);
}
