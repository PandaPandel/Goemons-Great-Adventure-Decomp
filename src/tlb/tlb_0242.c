#include "tlb.h"

extern ConnectData stage60PlacesEne0FAParam[];
ConnectData stage60PlacesEne0FAParam[] = {
    { 50.0f, 82, 1, 2440.0f, 240.0f, 0.0f, 0.0f, 0x0000, 0, 2 },
};
EneDef stage60Places[] = {
    { 0, 0, 0, 0, ENE_ETSK_KOBAN_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_DANGO_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_NEKO_MAIN_FLAGS_0, 0, 0 },
    { 0, 0, 0, 0, ENE_TB_HANI_TAMA_MAIN, 0, 0 },
    { 0, 0, 0, 0, ENE_BZ_HANI_TAMA, 0, 0 },
    { 0, 0, 0, 0, ENE_SIROBZ, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_BIWA_KUMO, 0, 0 },
    { 0, 0, 0, 0, ENE_ETSK_BIWA_FIRE, 0, 0 },
    { 1, 800, -59, 0, ENE_ETSK_BIWA_MAIN, 2, 0 },
    { 1, 850, -46, 0, ENE_SIRO_J_HOURS_ANY, 2, 0 },
    { 1, 900, -66, 0, ENE_SIRO_J_HOURS_ANY, 2, 1 },
    { 1, 990, -46, 0, ENE_ETSK_BIWA_MAIN, 2, 0 },
    { 0, 0, 0, 0, ENE_HAPPA, 2, 0 },
    { 0, 0, 10, 0, ENE_HAPPA_MASTER, 2, 0 },
    { 0, -200, -125, 0, ENE_WATER_RIDE_FLAGS_3, 2, 5 },
    { 1, 431, -200, 0, ENE_CONNECT, 2, (u32) &stage60PlacesEne0FAParam },
    { 1, 635, -80, 0, ENE_TRANSFER, 2, 0 },
    { 1, 1199, -71, 0, ENE_NEKOBUS_STOP, 0, 0 },
    { 0, 0, 0, 0, ENE_NEKOBUS, 0, 0 },
    { 0, 0, 0, 0, ENE_NEKOBUS_LIGHT, 0, 0 },
    { 0, 0, 0, 0, ENE_NEKOBUS_STOP_MARK, 0, 0 },
};

void stage60Build(void) {
    stageBuildBasic(0x3C, 1, 500.0f, -120.0f, 0.0f, (u32) stage60Places, 0x15);
    camAiSetFar(600);
    camSunSetProgram(6);
    camSunSetTunes5(0x518, 0x5E, 0x5F, 0x60, 0x61);
}
