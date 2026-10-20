#include "tlb.h"

extern ConnectData minnka1PlacesEne0FAParam[];
extern PersonData minnka1PlacesEne179Param[];
extern PersonData minnka1PlacesEne161Param[];
extern PersonData minnka1PlacesEne16AParam[];
extern PersonData minnka1PlacesEne15DParam[];
ConnectData minnka1PlacesEne0FAParam[] = {
    { 10.0f, 25, 0, 360.0f, 0.0f, -100.0f, 0.0f, 0x0000, 0, 0 },
};
PersonData minnka1PlacesEne179Param[] = {
    { 0, 0x00000000, { 4, 8 }, 0, 0.0f, 0 },
};
PersonData minnka1PlacesEne161Param[] = {
    { 442, 0x00000000, { 4, 8 }, 0, 0.0f, 0 },
};
PersonData minnka1PlacesEne16AParam[] = {
    { 551, 0x00000000, { 4, 8 }, 0, 0.0f, 0 },
};
PersonData minnka1PlacesEne15DParam[] = {
    { 445, 0x00000000, { 4, 8 }, 0, 0.0f, 0 },
};
EneDef minnka1Places[] = {
    { 0, 10, 0, 80, ENE_CONNECT, 2, (u32) &minnka1PlacesEne0FAParam },
    { 0, -17, 15, -35, ENE_HANAOYA, 2, (u32) &minnka1PlacesEne179Param },
    { 0, -32, 15, -5, ENE_BEAT_MANIA, 2, (u32) &minnka1PlacesEne161Param },
    { 0, -22, 15, 32, ENE_KINOKO_MANIA, 2, (u32) &minnka1PlacesEne16AParam },
    { 0, 28, 15, -18, ENE_TSUCHI_JISAN, 2, (u32) &minnka1PlacesEne15DParam },
    { 0, 45, 12, -30, ENE_OBISU_MARU, 0, 0 },
    { 0, 1, 0, 0, ENE_DEMO_AUTO, 2, 445 },
};

void minnka1Init(void) {
    stageBuildFixedCam(0x11, 0, 0.0f, 10.0f, 0.0f, (u32) minnka1Places, 7, 60.0f, 0.0f, 35.0f);
    camAiSetFar(1000);
}
